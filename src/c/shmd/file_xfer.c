/* Copyright (c) 2013 Antanas Uršulis <au231@cam.ac.uk>
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

#include "file_xfer.h"

#include <stdio.h>
#include <pthread.h>

void fxr_create(file_xfer *fxr)
{
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&fxr->lock, &mattr);
	pthread_mutexattr_destroy(&mattr);

	pthread_condattr_t cattr;
	pthread_condattr_init(&cattr);
	pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&fxr->ack, &cattr);
	pthread_condattr_destroy(&cattr);

	fxr_clear_flags(fxr);

	fprintf(stderr, "Initialised file_xfer struct\n");
}

void fxr_destroy(file_xfer *fxr)
{
	pthread_cond_destroy(&fxr->ack);
	pthread_mutex_destroy(&fxr->lock);
}

void fxr_clear_flags(file_xfer *fxr)
{
	fxr->idle = fxr->req_file = fxr->file_ok = fxr->file_noent =
		fxr->req_data = fxr->data_ok = fxr->data_left = fxr->end_xfer = 0;
}
