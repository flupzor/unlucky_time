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

#include <check.h>

#include "../src/unlucky_time.h"
#include "../src/utils.h"


START_TEST (test_unlucky_diff_first_of_month)
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

	ck_assert_int_eq(new_tm.tm_mday, 1);
}
END_TEST

START_TEST (test_unlucky_diff_last_of_month)
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

	ck_assert_int_eq(new_tm.tm_mday, days_in_month(new_tm.tm_mon, new_tm.tm_year));

}
END_TEST

START_TEST (test_unlucky_diff_leap_seconds)
{
	struct unlucky_state	state;
	time_t			start_time;

	memset(&state, 0, sizeof(state));

	// 2016-1-2 9:53:55
	start_time = 1451724835;

	unlucky_init(&state, start_time, UNLUCKY_LEAP_SECOND);

	ck_assert_int_eq(unlucky_diff(&state, start_time + 1), 0);  // 56
	ck_assert_int_eq(unlucky_diff(&state, start_time + 2), 0);  // 57
	ck_assert_int_eq(unlucky_diff(&state, start_time + 3), 0);  // 58
	ck_assert_int_eq(unlucky_diff(&state, start_time + 4), 0);  // 59
	ck_assert_int_eq(unlucky_diff(&state, start_time + 5), 0);  // 60
	ck_assert_int_eq(unlucky_diff(&state, start_time + 6), -1);  // 60 (again)
	ck_assert_int_eq(unlucky_diff(&state, start_time + 7), -1);  // 61
	ck_assert_int_eq(unlucky_diff(&state, start_time + 8), -1);  // 61
}
END_TEST

Suite * unlucky_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Unlucky");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_unlucky_diff_first_of_month);
    tcase_add_test(tc_core, test_unlucky_diff_last_of_month);
    tcase_add_test(tc_core, test_unlucky_diff_leap_seconds);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = unlucky_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
