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

#include "shm_object.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>

#define TRUNC(fd, type) ftruncate(fd, sizeof(type))
#define MAP_IN(type, fd) (type *)mmap(NULL, sizeof(type), \
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)

static shm_object *real_shm_object_attach(const char *name, object_type_t objtype, int create_object)
{
	shm_object *obj = (shm_object *)calloc(1, sizeof(shm_object));
	obj->shmname[0] = '/';
	strncpy(obj->shmname+1, name, NAME_MAX-1);
	obj->i_am_creator = create_object;

	obj->shmfd = shm_open(obj->shmname,
			create_object ? O_RDWR | O_CREAT : O_RDWR,
			S_IRUSR | S_IWUSR);

	if(obj->shmfd == -1) {
		perror("failed shm_open");
	}

	obj->objtype = objtype;
	switch(objtype) {
		case FILE_XFER:
			if(create_object) TRUNC(obj->shmfd, file_xfer);
			obj->obj.fxr = MAP_IN(file_xfer, obj->shmfd);
	}

	fprintf(stderr, "Attached to shm object %s, created: %d\n", obj->shmname, create_object);
	return obj;
}

shm_object *shm_object_create(const char *name, object_type_t objtype)
{
	return real_shm_object_attach(name, objtype, 1);
}

shm_object *shm_object_attach(const char *name, object_type_t objtype)
{
	return real_shm_object_attach(name, objtype, 0);
}

void shm_object_destroy(shm_object *obj)
{
	if(obj->shmfd >= 0) {
		close(obj->shmfd);
		if(obj->i_am_creator) shm_unlink(obj->shmname);
	}
	free(obj);
}
