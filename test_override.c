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

#include <sys/time.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "override.h"
#include "unlucky_time.h"
#include "utils.h"
#include "test.h"

void	verify_gettimeofday(void);
void	verify_time(void);
void	verify_clock_gettime(void);

time_t	consistent_time(void);
time_t	consistent_gettimeofday(void);
time_t	consistent_clock_gettime(void);


int
main(void)
{
	time_t time_r, gettimeofday_r, clock_gettime_r;

#ifdef OVERRIDE_GETTIMEOFDAY
	verify_gettimeofday();
#endif
#ifdef OVERRIDE_TIME
	verify_time();
#endif
#ifdef OVERRIDE_CLOCK_GETTIME
	verify_clock_gettime();
#endif

	/* Because we override various time function, and various libc
	 * implement these function different. For example, time() calls
	 * gettimeofday() on OpenBSD. So if we override both it would result in
	 * the diff being applied twice.
	 *
	 * So, here we check if all the time functions return the same value.
	 */

	time_r = consistent_time();
	gettimeofday_r = consistent_gettimeofday();
	clock_gettime_r = consistent_clock_gettime();

	ASSERT_TIME_EQUALS(time_r, gettimeofday_r);
	ASSERT_TIME_EQUALS(gettimeofday_r, clock_gettime_r);

	exit(EXIT_SUCCESS);
}

/*
 * Helper function to verify if the time(), gettimeofday and clock_gettime
 * functions return the same time.
 */

time_t
consistent_time(void)
{
	time_t	time_r;

	return time(NULL);
}

time_t
consistent_gettimeofday(void)
{
	int		r;
	struct timeval	rtval;

	r = gettimeofday(&rtval, NULL);
	if (r == -1) {
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}

	// We ignore tv_usec (microseconds)
	return rtval.tv_sec;
}

time_t
consistent_clock_gettime(void)
{
	struct timespec	tval;
	int		r;

	r = clock_gettime(CLOCK_REALTIME, &tval);
	if (r == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	// We ignore tv_usec (microseconds)
	return tval.tv_sec;
}

/*
 * Test wether a diff is applied on top of the original value returned by the
 * time(), gettimeofday() and clock_gettime() functions
 */
void
verify_time(void)
{
	time_t		time_r_orig, time_r_orig2, time_r, time_r2;
	time_t		diff;


	time_r2 = time(&time_r);
	assert(time_r2 == time_r);

	if (time_r2 == -1) {
		perror("time");
		exit(EXIT_FAILURE);
	}

	time_r_orig2 = original_time(&time_r_orig);
	assert(time_r_orig2 == time_r_orig);
	if (time_r_orig2 == -1) {
		perror("original_time");
		exit(EXIT_FAILURE);
	}

	diff = gettimediff(time_r_orig);
	if (diff == -1) {
		fprintf(stderr, "Failed to retrieve the time delta.");
		exit(EXIT_FAILURE);
	}

	ASSERT_TIME_DIFF(time_r, time_r_orig, diff);
}

void
verify_gettimeofday(void)
{
	struct timeval	tval_orig, tval;
	struct timezone	tzone_orig, tzone;
	int		r, r2;
	time_t		diff;

	r = gettimeofday(&tval, &tzone);
	if (r == -1) {
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}

	r2 = original_gettimeofday(&tval_orig, &tzone_orig);
	if (r2 == -1) {
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}

	diff = gettimediff(tval_orig.tv_sec);
	if (diff == -1) {
		fprintf(stderr, "Failed to retrieve the time delta.");
		exit(EXIT_FAILURE);
	}

	ASSERT_TIME_DIFF(tval.tv_sec, tval_orig.tv_sec, diff);
}

void
verify_clock_gettime(void)
{
	int		r, r2;
	time_t		diff;
	struct timespec	tval, tval_orig;

	r = clock_gettime(CLOCK_REALTIME, &tval);
	if (r == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	r2 = original_clock_gettime(CLOCK_REALTIME, &tval_orig);
	if (r == -1) {
		perror("original_clock_gettime");
		exit(EXIT_FAILURE);
	}

	diff = gettimediff(tval_orig.tv_sec);
	if (diff == -1) {
		fprintf(stderr, "Failed to retrieve the time delta.");
		exit(EXIT_FAILURE);
	}

	ASSERT_TIME_DIFF(tval.tv_sec, tval_orig.tv_sec, diff);
}
