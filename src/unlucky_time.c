/*
 * Copyright (c) 2016, 2017 Alexander Schrijver <alex@flupzor.nl
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

#include <assert.h>
#include <bsd/stdlib.h>
#include <err.h>
#include <stdio.h>
#include <time.h>

#include "unlucky_time.h"
#include "utils.h"

#define YEARS_IN_FUTURE 20

static time_t	first_of_month(time_t start_time);
static time_t	last_of_month(time_t start_time);
static time_t	leap_day(time_t start_time);
static time_t	dst_change(time_t start_time);
static int	leap_year(time_t start_time);
static time_t	nil(time_t start_time);

static int	future_year(time_t start_time);
static int	clock_changed(time_t start, time_t end);
static time_t	bisect(time_t start, time_t end);
static size_t find_dst_changes(time_t start_time, time_t *table, size_t size);

static time_t leap_seconds(time_t start_time, time_t current_time);
static time_t normal_seconds(time_t start_time, time_t current_time);


void
unlucky_init(struct unlucky_state *state, time_t start_time, enum unlucky_mode mode)
{
	struct {
		time_t			(*fn)(time_t);
		time_t			(*diff_fn)(time_t, time_t);
		enum unlucky_mode	 mode;
	} time_functions[] = {
		{first_of_month, normal_seconds, UNLUCKY_FIRST_OF_MONTH},
		{last_of_month, normal_seconds, UNLUCKY_LAST_OF_MONTH},
		{leap_day, normal_seconds, UNLUCKY_LEAP_DAY},
		{dst_change, normal_seconds, UNLUCKY_DST_CHANGE},
		{nil, leap_seconds, UNLUCKY_LEAP_SECOND},
	};
	size_t chosen_mode, mapping_size;

	if (state->initialized)
		return;

	mapping_size = sizeof(time_functions)/sizeof(time_functions[0]);

	if (mode == UNLUCKY_RANDOM)
		chosen_mode = arc4random_uniform(mapping_size);
	else
		chosen_mode = mode;

	state->start_time = start_time;
	state->initialized = 1;
	state->diff = time_functions[chosen_mode].fn(start_time) - start_time;
	state->diff_fn = time_functions[chosen_mode].diff_fn;
}

time_t
unlucky_diff(struct unlucky_state *state, time_t current_time)
{
	time_t start_time = state->start_time;
	return state->diff + state->diff_fn(start_time, current_time);
}

time_t
leap_seconds(time_t start_time, time_t current_time)
{
	time_t delta, offset, leap_seconds;
	delta = current_time - start_time;

	// Every minute in the delta 1 leap second should be subtracted.
	// The offset deals with 60 happening twice nicely, and not 59, or something else.
	offset = ((delta/60) % 60 - start_time % 60)  % 60;
	leap_seconds = (delta-offset-1) / 60;

	return -leap_seconds;
}

static time_t
normal_seconds(time_t start_time, time_t current_time)
{
	return 0;
}


static int
leap_year(time_t start_time)
{
	int tm_year;

	do {
		tm_year = future_year(start_time);
	} while (! is_leap_year(tm_year));

	return tm_year;
}

static int
future_year(time_t start_time)
{
	int		random_offset, tm_year, current_year;
	struct timespec	ts;
	struct tm	tm;

	random_offset = arc4random_uniform(YEARS_IN_FUTURE);

	if (gmtime_r(&start_time, &tm) == NULL)
		err(1, "gmtime_r");

	return tm.tm_year + random_offset;
}

/* Return a randomized struct tm which at its latests
 * is 1 hour before the end of the day (23:00).
 */
static void
random_tm(struct tm *tm)
{
	int tm_year, tm_month, tm_mday;

	tm_year = arc4random_uniform(YEARS_IN_FUTURE);
	tm_month = arc4random_uniform(12);
	tm_mday = arc4random_uniform(days_in_month(tm_month, tm_year));

	tm->tm_sec = arc4random_uniform(59);
	tm->tm_min = arc4random_uniform(59);
	tm->tm_hour = arc4random_uniform(22);
	tm->tm_mday = tm_mday;
	tm->tm_mon = tm_month;
	tm->tm_year = tm_year;
	tm->tm_isdst = 0;
}

static time_t
first_of_month(time_t start_time)
{
	struct tm	first_of_month;

	random_tm(&first_of_month);

	first_of_month.tm_mday = 1;
	first_of_month.tm_mon = arc4random_uniform(12);
	first_of_month.tm_year = future_year(start_time);

	return mktime(&first_of_month);
}

static time_t
last_of_month(time_t start_time)
{
	struct tm	last_of_month;
	int		tm_month, tm_year;

	tm_year = future_year(start_time);
	tm_month = arc4random_uniform(12);

	random_tm(&last_of_month);

	last_of_month.tm_mday = days_in_month(tm_month, tm_year);
	last_of_month.tm_mon = tm_month;
	last_of_month.tm_year = tm_year;

	return mktime(&last_of_month);
}

static time_t
leap_day(time_t start_time)
{
	struct tm	leap_day;
	int tm_year = leap_year(start_time);

	random_tm(&leap_day);

	leap_day.tm_mday = 29;
	leap_day.tm_mon = 2;
	leap_day.tm_year = tm_year;

	return mktime(&leap_day);
}

/*
 * The wall clock was changed by dailight saving time change.
 */
static int
clock_changed(time_t start, time_t end)
{
	struct tm start_tm, end_tm;

	if (localtime_r(&start, &start_tm) == NULL)
		err(1, "localtime_r");
	if (localtime_r(&end, &end_tm) == NULL)
		err(1, "localtime_r");

	return start_tm.tm_isdst != end_tm.tm_isdst;
}

static time_t
clock_delta(time_t start, time_t end)
{
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define isleap_sum(a, b)        isleap((a) % 400 + (b) % 400)
#define DAYSPERNYEAR    365
#define HOURSPERDAY     24
#define TM_YEAR_BASE    1900
#define MINSPERHOUR     60
#define SECSPERMIN      60
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((int_fast32_t) SECSPERHOUR * HOURSPERDAY)

	struct tm start_tm, end_tm;
	time_t    local_start, local_end;
        long      result;
        int       tmy;

	if (localtime_r(&start, &start_tm) == NULL)
		err(1, "localtime_r");
	if (localtime_r(&end, &end_tm) == NULL)
		err(1, "localtime_r");

        if (end_tm.tm_year < start_tm.tm_year)
                return -clock_delta(start, end);
        result = 0;
        for (tmy = start_tm.tm_year; tmy < end_tm.tm_year; ++tmy)
                result += DAYSPERNYEAR + isleap_sum(tmy, TM_YEAR_BASE);
        result += end_tm.tm_yday - start_tm.tm_yday;
        result *= HOURSPERDAY;
        result += end_tm.tm_hour - start_tm.tm_hour;
        result *= MINSPERHOUR;
        result += end_tm.tm_min - start_tm.tm_min;
        result *= SECSPERMIN;
        result += end_tm.tm_sec - start_tm.tm_sec;
        return result;
}

static time_t
bisect(time_t start, time_t end)
{
	time_t diff, midpoint;

	for (;;) {
		diff = end - start;
		if (diff < 2)
			break;
		midpoint = start + (diff / 2);

		assert(midpoint > start);
		assert(midpoint < end);

		if (clock_changed(start, midpoint))
			end = midpoint;
		else
			start = midpoint;
	}

	return start;
}

static size_t
find_dst_changes(time_t start_time, time_t *table, size_t size)
{
	time_t start, end, current, next, change;
	size_t i = 0;

	tzset();

	start = start_time;
	end = start + (60 * 60 * 24 * 365 * 20);

	for (current = start; current < end; ) {
		next = current + (60 * 60 * 12);

		if (clock_changed(current, next)) {
			change = bisect(current, next);
			if (i >= size)
				break;

			table[i++] = change;
		}

		current = next;
	}

	return i;
}

static time_t
dst_change(time_t start_time)
{
	time_t dst_changes[500], start, end, start_change, delta;
	size_t size, i;

	size = find_dst_changes(start_time, dst_changes, sizeof *dst_changes);
	i = arc4random_uniform(size);

	start = dst_changes[i];
	end = start + 1;
	delta = clock_delta(start, end);
	if (delta > 0) {
		/* The wall clock goes backwards */
		start -= 60 * 1;
	} else {
		/* The wall clock goes backwards, e.g. if the clock would be set back at 02:59:59
		 * to 02:00:00 then 02:00:00 -> 02:59:59 would happen twice on the clock.
		 *
		 * This is where certain programs break.
		 *
		 * Try setting the time somewhere in the period of 4 minutes before the time change
		 * until half until the repeated period (~30 minutes in this case)
		 */
		start -= (60 * 4);
		start += arc4random_uniform(((-delta)/2) + (60 * 4));
	}

	return start;
}


static time_t
nil(time_t start_time)
{
	return start_time;
}
