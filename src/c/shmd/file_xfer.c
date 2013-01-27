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
