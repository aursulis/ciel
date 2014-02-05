/* Copyright (c) 2013 Antanas Uršulis <antanas@cantab.net>
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

#include "shm_fs.h"
#include "shm_fs_fifos.h"

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

bool is_present_in_shmfs(const char *refname)
{
	return shmfs_lookup(basename(refname)) != MAGIC_INVALID_ENTRY;
}

void open_for_reading(const char *refname, char *shmname)
{
	shmfs_get_read_filename(basename(refname), shmname);
}

void open_for_writing(const char *refname, char *shmname)
{
	shmfs_get_write_filename(basename(refname), shmname);
}

bool load_into_shmfs(const char *refname)
{
	return shmfs_load_local(refname) != MAGIC_INVALID_ENTRY;
}

void perform_commit(const char *oldname, const char *newname)
{
	shmfs_commit(basename(oldname), basename(newname));
}

off_t get_file_size(const char *refname)
{
	return shmfs_getsize(basename(refname));
}
