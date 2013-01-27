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
