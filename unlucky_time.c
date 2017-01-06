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
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "unlucky_time.h"


int	diff_set;
time_t	diff;

void _init_time_diff(void);

//#define OVERRIDE_TIME 1
//#define OVERRIDE_GETTIMEOFDAY 1

void
unlucky_init(void)
{
	_init_time_diff();

	if (original_gettimeofday == NULL)
		original_gettimeofday = (gettimeofday_func_t)dlsym(RTLD_NEXT, "gettimeofday");

	if (original_time == NULL)
		original_time = (time_func_t)dlsym(RTLD_NEXT, "time");

	if (original_clock_gettime == NULL)
		original_clock_gettime = (clock_gettime_func_t)dlsym(RTLD_NEXT, "clock_gettime");
}

void
_init_time_diff(void)
{
	int time_period_in_seconds;

	// 31st
	// 1st
	// 2014 -> 2015
	// year -> year+1
	// leap day


	// Initialize the diff one time to make sure the time does continue
	// normally.
	if (diff_set == 0) {
		// 6 years, 3 years in the past and 3 years in the future.
		time_period_in_seconds = 6 * 365 * 24 * 60 * 60;

		diff = arc4random_uniform(time_period_in_seconds);

		diff -= time_period_in_seconds / 2;

		diff_set = 1;
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
