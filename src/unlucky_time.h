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

enum unlucky_mode {
	UNLUCKY_FIRST_OF_MONTH,
	UNLUCKY_LAST_OF_MONTH,
	UNLUCKY_LEAP_DAY,
	UNLUCKY_DST_CHANGE,
	UNLUCKY_LEAP_SECOND,
	UNLUCKY_RANDOM,
};

struct unlucky_state {
	int	initialized;
	time_t  (*diff_fn)(time_t, time_t);
	time_t  start_time;
	time_t	diff;
};

void	unlucky_init(struct unlucky_state *state, time_t start_time, enum unlucky_mode mode);
time_t	unlucky_diff(struct unlucky_state *state, time_t current_time);

