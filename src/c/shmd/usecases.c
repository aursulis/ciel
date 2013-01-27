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
#include "file_xfer.h"
#include "file_xfer_client.h"
#include "file_xfer_server.h"

void fxr_server_main(const char *shmname)
{
	shm_object *shmobj = shm_object_create(shmname, FILE_XFER);
	fxr_create(shmobj->obj.fxr);
	file_xfer_server(shmobj->obj.fxr);
	fxr_destroy(shmobj->obj.fxr);
	shm_object_destroy(shmobj);
}

void fxr_client_main(const char *src_host, const char *src_file, const char *dest_file)
{
	shm_object *shmobj = shm_object_attach(src_host, FILE_XFER);
	file_xfer_client(shmobj->obj.fxr, src_file, dest_file);
	shm_object_destroy(shmobj);
}
