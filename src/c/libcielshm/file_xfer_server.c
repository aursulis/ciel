#include "file_xfer.h"

#include <errno.h>
#include <stdio.h>
#include <pthread.h>

static FILE *xfer_fp;

static void wait_datareq(file_xfer *fxr)
{
	while(1) {
		fprintf(stderr, "Waiting for data request\n");
		while(!fxr->req_data && !fxr->end_xfer) {
			pthread_cond_wait(&fxr->ack, &fxr->lock);
		}
		if(fxr->end_xfer) {
			fprintf(stderr, "Got end request\n");
			fclose(xfer_fp);
			return;
		} else {
			fprintf(stderr, "Got data request\n");
			fxr->bytes_written = fread(fxr->buffer, sizeof(char), BUFSIZE, xfer_fp);
			if(fxr->bytes_written == 0) {
				if(!feof(xfer_fp)) {
					fprintf(stderr, "Read error\n");
					fxr->data_ok = 0;
					fxr->end_xfer = 1;
					pthread_cond_broadcast(&fxr->ack);
					fclose(xfer_fp);
					return;
				}
				fprintf(stderr, "No data\n");
			}
			fxr->data_ok = 1;
			fxr->data_left = feof(xfer_fp) ? 0 : 1;
			fxr->req_data = 0;
			fprintf(stderr, "Read %d bytes, data_left set to %d\n", fxr->bytes_written, fxr->data_left);
			pthread_cond_broadcast(&fxr->ack);
		}
	}
}

static void init_xfer(file_xfer *fxr)
{
	fprintf(stderr, "File is %s\n", fxr->buffer);
	xfer_fp = fopen(fxr->buffer, "rb");
	if(xfer_fp == NULL) {
		fprintf(stderr, "Could not open\n");
	   	if(errno == ENOENT) {
			fxr->file_noent = 1;
		}
		fxr->file_ok = 0;
		fxr->end_xfer = 1;
		pthread_cond_broadcast(&fxr->ack);
	} else {
		fprintf(stderr, "File opened\n");
		fxr->file_ok = 1;
		fxr->data_left = 1;
		pthread_cond_broadcast(&fxr->ack);
		wait_datareq(fxr);
	}
}

static void wait_req(file_xfer *fxr)
{
	pthread_mutex_lock(&fxr->lock);
	while(1) {
		fprintf(stderr, "Waiting for file request\n");
		fxr_clear_flags(fxr);
		fxr->idle = 1;
		while(!fxr->req_file) {
			pthread_cond_wait(&fxr->ack, &fxr->lock);
		}
		fprintf(stderr, "Got file request\n");
		fxr->req_file = 0;
		init_xfer(fxr);
	}
}

void file_xfer_server(file_xfer *fxr)
{
	wait_req(fxr);
}
