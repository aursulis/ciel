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
#include "shm_fs_arch.h"

#ifndef KERN_LINUX
	#error "Available only on normal Linux"
#endif

extern struct shmfs *fs;

#include <stdio.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
	shmfs_init(1);

	get_dir_lock();
	get_inodes_lock();

	for(int i = 0; i < SHMFS_NFILES; ++i) {
		int iid = fs->files[i].inode_id;
		if(iid != MAGIC_INVALID_ENTRY) {
			off_t size = fs->inodes[iid].size;
			printf("%6d %10d %s\n", iid, size, fs->files[i].name);
		}
	}

	release_inodes_lock();
	release_dir_lock();

	return 0;
}
