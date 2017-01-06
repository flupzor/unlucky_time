
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


typedef int (*gettimeofday_func_t)(struct timeval *tp, struct timezone *tzp);
typedef int (*time_func_t)(time_t *t);
typedef int (*clock_gettime_func_t)(clockid_t clock_id, struct timespec *tp);
typedef int (*connect_func_t)(int s, const struct sockaddr *name, socklen_t namelen);

gettimeofday_func_t	original_gettimeofday;
time_func_t		original_time;
clock_gettime_func_t	original_clock_gettime;

time_t			gettimediff(time_t current_time);
