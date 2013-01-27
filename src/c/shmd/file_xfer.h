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

#ifndef FILE_XFER_H
#define FILE_XFER_H

#include <pthread.h>

#define BUFSIZE 4096

typedef struct file_xfer {
	pthread_mutex_t lock;
	pthread_cond_t ack;
	unsigned idle       : 1;
	unsigned req_file   : 1;
	unsigned file_ok    : 1;
	unsigned file_noent : 1;
	unsigned req_data   : 1;
	unsigned data_ok    : 1;
	unsigned data_left  : 1;
	unsigned end_xfer   : 1;
	size_t bytes_written;
	char buffer[BUFSIZE];
} file_xfer;

void fxr_create(file_xfer *fxr);
void fxr_destroy(file_xfer *fxr);
void fxr_clear_flags(file_xfer *fxr);
#endif
