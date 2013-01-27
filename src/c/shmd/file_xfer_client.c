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

#include <string.h>
#include <stdio.h>
#include <pthread.h>

static FILE *xfer_fp;

static void copy_loop(file_xfer *fxr, const char *dest_file)
{
	fprintf(stderr, "Opening destination file\n");
	xfer_fp = fopen(dest_file, "wb");
	while(fxr->data_left && !fxr->end_xfer) {
		fxr_clear_flags(fxr);
		fxr->req_data = 1;
		pthread_cond_broadcast(&fxr->ack);
		fprintf(stderr, "Request for data\n");
		while(fxr->req_data) {
			pthread_cond_wait(&fxr->ack, &fxr->lock);
		}
		if(!fxr->data_ok) {
			fprintf(stderr, "Data not ok\n");
			fclose(xfer_fp);
			return;
		}
		fprintf(stderr, "Got %d bytes\n", fxr->bytes_written);
		fwrite(fxr->buffer, fxr->bytes_written, 1, xfer_fp);
	}
	if(!fxr->end_xfer) {
		fxr->end_xfer = 1;
		pthread_cond_broadcast(&fxr->ack);
	}
	fclose(xfer_fp);
}

static void make_req(file_xfer *fxr, const char *src_file, const char *dest_file)
{
	pthread_mutex_lock(&fxr->lock);
	fprintf(stderr, "Waiting for idle\n");
	while(!fxr->idle) {
		pthread_cond_wait(&fxr->ack, &fxr->lock);
	}
	fxr->idle = 0;
	fxr->req_file = 1;
	strncpy(fxr->buffer, src_file, BUFSIZE);

	pthread_cond_broadcast(&fxr->ack);
	fprintf(stderr, "Requesting transfer of %s\n", src_file);
	while(fxr->req_file) {
		pthread_cond_wait(&fxr->ack, &fxr->lock);
	}
	if(fxr->end_xfer || !fxr->file_ok) {
		fprintf(stderr, "File not ok\n");
		return;
	}
	fprintf(stderr, "File ok\n");
	copy_loop(fxr, dest_file);
	pthread_mutex_unlock(&fxr->lock);
}

void file_xfer_client(file_xfer *fxr, const char *src_file, const char *dest_file)
{
	make_req(fxr, src_file, dest_file);
}
