/*
 * Copyright (c) 2017 Alexander Schrijver <alex@flupzor.nl
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

#include <sys/time.h>
#include <time.h>
#include <err.h>

#include "override.h"
#include "utils.h"

time_t
current_time(void)
{
	struct timespec		ts;
	struct tm		tm;
	clock_gettime_func_t	_clock_gettime = clock_gettime;

	if (original_clock_gettime)
		_clock_gettime = original_clock_gettime;

	if (_clock_gettime(CLOCK_REALTIME, &ts) == -1)
		err(1, "clock_gettime");

	return ts.tv_sec;
}

int
days_in_month(int tm_month, int tm_year)
{
	int monthtodays[12] = {
		31, // Januari
		28, // Februari
		31, // March
		30, // April
		31, // May
		30, // June
		31, // Juli
		31, // August
		30, // September
		31, // October
		30, // November
		31, // December
	};

	if (is_leap_year(tm_year))
		monthtodays[1] = 29;

	return monthtodays[tm_month];
}

int
is_leap_year(int tm_year)
{
	int year = tm_year + 1900;

	if ((year % 4) != 0)
		return 0; // common year
	else if ((year % 100) != 0)
		return 1; // leap year
	else if ((year % 400) != 0)
		return 0; // common year
	else
		return 1; // leap year
}
