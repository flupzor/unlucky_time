/*
 * Copyright (c) 2015 Alexander Schrijver <alex@flupzor.nl
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE

#include <dlfcn.h>

#include <time.h>
#include <tzfile.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>

#include "unlucky_time.h"


int	diff_set;
time_t	diff;

void _init_time_diff(void);

//#define OVERRIDE_TIME 1
//#define OVERRIDE_GETTIMEOFDAY 1

void
unlucky_init(void)
{
	if (original_gettimeofday == NULL)
		original_gettimeofday = (gettimeofday_func_t)dlsym(RTLD_NEXT, "gettimeofday");

	if (original_time == NULL)
		original_time = (time_func_t)dlsym(RTLD_NEXT, "time");

	if (original_clock_gettime == NULL)
		original_clock_gettime = (clock_gettime_func_t)dlsym(RTLD_NEXT, "clock_gettime");

	_init_time_diff();
}

enum time_mode {
	UL_RANDOM_TIME = 0,
	UL_YEAR_CHANGE = 1,
	UL_MONTH_CHANGE_FIRST_DAY = 2,
	UL_MONTH_CHANGE_LAST_DAY = 3,
	UL_LEAP_DAY = 4,
	UL_NR_ENTRIES = 5,
};

int
_days_in_month(int year, int month)
{
	switch (month) {
	case TM_JANUARY:
		return 31;
	case TM_FEBRUARY:
		// Years that are divisible by 100 but not
		// by 400 do not contain leap days.
		if (year % 100 == 0 && year % 400 != 0)
			return 28;

		if (year % 4 == 0)
			return 29;
		return 28;
	case TM_MARCH:
		return 31;
	case TM_APRIL:
		return 30;
	case TM_MAY:
		return 31;
	case TM_JUNE:
		return 30;
	case TM_JULY:
		return 31;
	case TM_AUGUST:
		return 31;
	case TM_SEPTEMBER:
		return 30;
	case TM_OCTOBER:
		return 31;
	case TM_NOVEMBER:
		return 30;
	case TM_DECEMBER:
		return 31;
	}
}

/* The function find a random date which is higher than the date
 * specified in the current parameter. This date is then written
 * to the new parameter.
 *
 * The new date written to the new parameter are randomly picked
 * when the value is -1 or is not modified when the value is
 * any other value.
 *
 * Returns a time difference from the current time.
 */
time_t
_random_time_in_the_future(struct tm *current, struct tm *new)
{
	int days_in_month;

	if (new->tm_year == -1)
		new->tm_year = current->tm_year + arc4random_uniform(6 + 1); // Max 6 years in the future.

	assert (new->tm_year >= current->tm_year);

	if (new->tm_mon == -1) {
		// Check if this is this year.
		if (new->tm_year == current->tm_year) {
			new->tm_mon = current->tm_mon + arc4random_uniform(11 + 1-current->tm_mon);
		} else {
			new->tm_mon = arc4random_uniform(11 + 1);
		}
	}

	assert (new->tm_mon <= 11 && new->tm_mon >= 0);

	days_in_month = _days_in_month(new->tm_year + 1900, new->tm_mon);

	if (new->tm_mday == -1) {
		if (new->tm_year == current->tm_year && new->tm_mon == current->tm_mon) {
			printf("p1\n");
			new->tm_mday = current->tm_mday + arc4random_uniform(days_in_month + 1 - current->tm_mday);
		} else {
			printf("p2\n");
			new->tm_mday = arc4random_uniform(days_in_month) + 1;
		}
	}

	assert (new->tm_mday > 0 && new->tm_mday <= days_in_month);

	if (new->tm_hour == -1) {
		// Check this is this day.
		if (new->tm_year == current->tm_year && new->tm_mon == current->tm_mon &&
		    new->tm_mday == current->tm_mday) {
			new->tm_hour = current->tm_hour + arc4random_uniform(23 + 1 - current->tm_hour);
		} else {
			new->tm_hour = arc4random_uniform(23 + 1);
		}
	}

	assert(new->tm_hour >= 0 && new->tm_hour <= 23);

	if (new->tm_min == -1 ) {
		// Check if this is this hour.
		if (new->tm_year == current->tm_year && new->tm_mon == current->tm_mon &&
		    new->tm_mday == current->tm_mday && new->tm_hour == current->tm_hour) {
			new->tm_min = current->tm_min + arc4random_uniform(59 + 1 - current->tm_min);
		} else {
			new->tm_min = arc4random_uniform(59 + 1);
		}
	}

	assert(new->tm_hour >= 0 && new->tm_hour <= 59);

	if (new->tm_sec == -1) {
		// Check if this is this minute.
		// TODO: This can be 60 in case of a LEAP second.
		if (new->tm_year == current->tm_year && new->tm_mon == current->tm_mon &&
		    new->tm_mday == current->tm_mday && new->tm_hour == current->tm_hour &&
		    new->tm_min == current->tm_min) {
			new->tm_sec = current->tm_sec + arc4random_uniform(59 + 1 - current->tm_sec);
		} else {
			new->tm_sec = arc4random_uniform(59 + 1);
		}
	}

	assert(new->tm_hour >= 0 && new->tm_hour <= 59);

	// Nake sure it is in the future.
	assert(mktime(new) >= mktime(current));

	new->tm_isdst = current->tm_isdst;
	new->tm_gmtoff = current->tm_gmtoff;
	new->tm_zone = current->tm_zone;


	return mktime(new) - mktime(current);
}

int
_month_change_last_day(struct tm *current, struct tm *new)
{
	new->tm_year = current->tm_year + arc4random_uniform(6 + 1); // Max 6 years in the future.

	// Check if this is this year.
	if (new->tm_year == current->tm_year) {
		new->tm_mon = current->tm_mon + arc4random_uniform(11 + 1-current->tm_mon);
	} else {
		new->tm_mon = arc4random_uniform(11 + 1);
	}

	new->tm_mday = _days_in_month(new->tm_year + 1900, new->tm_mon);
	new->tm_hour = -1;
	new->tm_min = -1;
	new->tm_sec = -1;
	new->tm_isdst = 0;
	new->tm_gmtoff = 0;
	new->tm_zone = NULL;

	return _random_time_in_the_future(current, new);
}

int
_month_change_first_day(struct tm *current, struct tm *new)
{
	new->tm_year = -1;
	new->tm_mon = -1;
	new->tm_hour = -1;
	new->tm_min = -1;
	new->tm_sec = -1;
	new->tm_isdst = 0;
	new->tm_gmtoff = 0;
	new->tm_zone = NULL;

	new->tm_mday = 1;

	return _random_time_in_the_future(current, new);
}

int
_leap_day(struct tm *current, struct tm *new)
{
	int year;

	year = current->tm_year + 1900;

	// Either, if the current is a leap year, select that,
	// otherwise select the next leap year.
	year = year + 4 - (year % 4);

	// randomly pick max 3 * 4 = 12 years in the future.
	year += 4 * arc4random_uniform(3 + 1);

	// Years that are divisible by 100 but not
	// by 400 do not contain leap days.
	if (year % 100 == 0 && year % 400 != 0)
		year += 4;

	new->tm_year = year - 1900;
	new->tm_mon = TM_FEBRUARY;
	new->tm_mday = 29;
	new->tm_hour = -1;
	new->tm_min = -1;
	new->tm_sec = -1;

	return _random_time_in_the_future(current, new);
}

int
_year_change(struct tm *current, struct tm *new)
{
	new->tm_year = current->tm_year + arc4random_uniform(6 + 1); // Max 6 years in the future.
	new->tm_mon = TM_DECEMBER;
	new->tm_mday = _days_in_month(new->tm_year + 1900, new->tm_mon);
	new->tm_hour = -1;
	new->tm_min = -1;
	new->tm_sec = -1;

	return _random_time_in_the_future(current, new);
}

void
_init_time_diff(void)
{
	int		time_period_in_seconds;
	enum time_mode	mode;
	struct timespec	current_time;

	mode = arc4random_uniform(UL_NR_ENTRIES);

	if (diff_set == 1)
		return;

	if (original_clock_gettime(CLOCK_REALTIME, &current_time) == -1)
		abort();

	switch(mode) {
	case UL_RANDOM_TIME:
		// 6 years in the future;
		time_period_in_seconds = 6 * 365 * 24 * 60 * 60;

		diff = arc4random_uniform(time_period_in_seconds);
		diff_set = 1;
	break;
	case UL_MONTH_CHANGE_LAST_DAY: {
		struct tm	new, cur;
		int		year;

		if (gmtime_r(&current_time.tv_sec, &cur) == NULL)
			abort();

		diff = _month_change_last_day(&cur, &new);
		diff_set = 1;
		break;
	}
	case UL_MONTH_CHANGE_FIRST_DAY: {
		struct tm	new, cur;
		int		year;

		if (gmtime_r(&current_time.tv_sec, &cur) == NULL)
			abort();

		diff = _month_change_first_day(&cur, &new);
		diff_set = 1;
		break;
	}
	case UL_YEAR_CHANGE: {
		struct tm	new, cur;
		int		year;

		if (gmtime_r(&current_time.tv_sec, &cur) == NULL)
			abort();

		diff = _year_change(&cur, &new);
		diff_set = 1;
		break;
	}
	case UL_LEAP_DAY: {
		struct tm	new, cur;
		int		year;

		if (gmtime_r(&current_time.tv_sec, &cur) == NULL)
			abort();

		diff = _leap_day(&cur, &new);
		diff_set = 1;

		break;
		}
	}
}

time_t
gettimediff(void)
{
	unlucky_init();

	return diff;
}

#ifdef OVERRIDE_CLOCK_GETTIME
int
clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	int r;

	unlucky_init();

	r = original_clock_gettime(clock_id, tp);

	if (r == 0 && clock_id == CLOCK_REALTIME) {
		tp->tv_sec += diff;
	}

	return r;
}
#endif

#ifdef OVERRIDE_GETTIMEOFDAY
int
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	int			r;
	struct tm		res;

	unlucky_init();

	r = original_gettimeofday(tp, tzp);

	if (r == 0) {
		tp->tv_sec += diff;
	}

	return r;
}
#endif

// XXX: On OpenBSD time(3) calls gettimeofday(2) not sure if that's the case on
// Linux as well.
#ifdef OVERRIDE_TIME
time_t
time(time_t *tloc)
{
	time_t		r;

	unlucky_init();

	r = original_time(tloc);
	if (r == -1)
		return r;

	if (tloc) {
		(*tloc) += diff;
	}

	r += diff;

	return r;
}
#endif

int
connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	struct sockaddr_in	*i_addr;
	connect_func_t		 original_connect;

	original_connect = (connect_func_t)dlsym(RTLD_NEXT,"connect");

	if (name->sa_family == AF_INET) {
		i_addr = (struct sockaddr_in *)name;

		if (i_addr->sin_addr.s_addr != inet_addr("127.0.0.1")) {
			errno = EHOSTUNREACH;

			return -1;
		}
	}

	return original_connect(s, name, namelen);
}
