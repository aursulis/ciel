#ifndef SHM_OBJECT_H
#define SHM_OBJECT_H

#include "file_xfer.h"
#include <linux/limits.h>

typedef enum { FILE_XFER } object_type_t;
typedef struct shm_object {
	int shmfd;
	char shmname[NAME_MAX+1];
	int i_am_creator;
	object_type_t objtype;
	union {
		file_xfer *fxr;
	} obj;
} shm_object;

shm_object *shm_object_create(const char *name, object_type_t objtype);
shm_object *shm_object_attach(const char *name, object_type_t objtype);
void shm_object_destroy(shm_object *obj);

#endif
