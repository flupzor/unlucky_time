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

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <stdlib.h>

#include "unlucky_time.h"
#include "test.h"
#include "utils.h"

void test_unlucky_diff_leap_seconds(void);
void test_unlucky_diff_first_of_month(void);
void test_unlucky_diff_last_of_month(void);

int
main(void)
{
	test_unlucky_diff_leap_seconds();
	test_unlucky_diff_first_of_month();
	test_unlucky_diff_last_of_month();

	exit(EXIT_SUCCESS);
}

void
test_unlucky_diff_first_of_month(void)
{
	struct unlucky_state	state;
	time_t			start_time, new_time;
	struct tm		new_tm;

	memset(&state, 0, sizeof(state));

	// 2016-1-2 9:53:55
	start_time = 1451724835;

	unlucky_init(&state, start_time, UNLUCKY_FIRST_OF_MONTH);
	new_time = start_time + unlucky_diff(&state, start_time);
	if (localtime_r(&new_time, &new_tm) == NULL)
		err(1, "localtime_r");

	ASSERT_INT_EQUALS(new_tm.tm_mday, 1);
}

void
test_unlucky_diff_last_of_month(void)
{
	struct unlucky_state	state;
	time_t			start_time, new_time;
	struct tm		new_tm;

	memset(&state, 0, sizeof(state));

	// 2016-1-2 9:53:55
	start_time = 1451724835;

	unlucky_init(&state, start_time, UNLUCKY_LAST_OF_MONTH);
	new_time = start_time + unlucky_diff(&state, start_time);
	if (localtime_r(&new_time, &new_tm) == NULL)
		err(1, "localtime_r");

	ASSERT_INT_EQUALS(new_tm.tm_mday, days_in_month(new_tm.tm_mon, new_tm.tm_year));
}

void
test_unlucky_diff_leap_seconds(void)
{
	struct unlucky_state	state;
	time_t			start_time;

	memset(&state, 0, sizeof(state));

	// 2016-1-2 9:53:55
	start_time = 1451724835;

	unlucky_init(&state, start_time, UNLUCKY_LEAP_SECOND);

	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 1), 0);  // 56
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 2), 0);  // 57
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 3), 0);  // 58
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 4), 0);  // 59
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 5), 0);  // 60
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 6), -1);  // 60 (again)
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 7), -1);  // 61
	ASSERT_TIME_EQUALS(unlucky_diff(&state, start_time + 8), -1);  // 61
}
