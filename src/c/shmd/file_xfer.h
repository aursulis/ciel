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
