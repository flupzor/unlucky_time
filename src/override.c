/*
 * Copyright (c) 2015, 2016, 2017 Alexander Schrijver <alex@flupzor.nl
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

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "unlucky_time.h"
#include "override.h"
#include "utils.h"


static int			_time_entered;
static struct unlucky_state	state;

static void _init_time_diff(void);
static void _cleanup_time(void);

#if DEBUG
#define DPRINTF(args...) fprintf(stderr, args)
#else
#define DPRINTF(args...) do {} while(0)
#endif

static void
_init_time(void)
{
	// Make sure we never enter one of the functions we override.
	assert(_time_entered == 0);
	_time_entered = 1;

	if (original_gettimeofday == NULL)
		original_gettimeofday = (gettimeofday_func_t)dlsym(RTLD_NEXT, "gettimeofday");

	if (original_time == NULL)
		original_time = (time_func_t)dlsym(RTLD_NEXT, "time");

	if (original_clock_gettime == NULL)
		original_clock_gettime = (clock_gettime_func_t)dlsym(RTLD_NEXT, "clock_gettime");

	unlucky_init(&state, current_time(), UNLUCKY_RANDOM);
}

static void
_cleanup_time(void)
{
	_time_entered = 0;
}

time_t
gettimediff(time_t current_time)
{
	time_t diff;
	_init_time();
	diff = unlucky_diff(&state, current_time);
	_cleanup_time();
	return diff;
}

#ifdef OVERRIDE_CLOCK_GETTIME
int
clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	int	r;
	time_t	diff = 0;

	_init_time();

	r = original_clock_gettime(clock_id, tp);

	if (r == 0 && clock_id == CLOCK_REALTIME) {
		diff = unlucky_diff(&state, tp->tv_sec);
		tp->tv_sec += diff;
	}

	DPRINTF("clock_gettime date returned: %s (diff added: %lu)\n", asctime(localtime(&tp->tv_sec)), diff);

	_cleanup_time();

	return r;
}
#endif

#ifdef OVERRIDE_GETTIMEOFDAY
int
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	int		r;
	struct tm	res;
	time_t		diff;

	_init_time();

	r = original_gettimeofday(tp, tzp);

	if (r == 0) {
		diff = unlucky_diff(&state, tp->tv_sec);
		tp->tv_sec += diff;
	}

	DPRINTF("gettimeofday date returned: %s (diff added: %lu)\n", asctime(localtime(&tp->tv_sec)), diff);

	_cleanup_time();

	return r;
}
#endif

#ifdef OVERRIDE_TIME
time_t
time(time_t *tloc)
{
	time_t		r;

	_init_time();

	r = original_time(tloc);
	if (r == -1)
		return r;

	if (tloc) {
		(*tloc) += unlucky_diff(&state, r);
	}

	r += unlucky_diff(&state, r);

	DPRINTF("time date returned: %s\n", asctime(localtime(&r)));

	_cleanup_time();

	return r;
}
#endif
