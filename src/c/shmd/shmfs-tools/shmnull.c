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

int main(int argc, char **argv)
{
	shmfs_init(1);

	get_stats_lock();
	get_dir_lock();
	get_inodes_lock();
	get_fat_lock();

	fs->stats.nwrites = 0;
	fs->stats.free_dirents = SHMFS_NFILES;
	fs->stats.free_inodes = SHMFS_NINODES;
	fs->stats.free_blocks = SHMFS_NBLOCKS;

	for(int i = 0; i < SHMFS_NFILES; ++i) {
		fs->files[i].inode_id = MAGIC_INVALID_ENTRY;
	}

	for(int i = 0; i < SHMFS_NINODES; ++i) {
		fs->inodes[i].flags.valid = 0;
	}

	for(int i = 0; i < SHMFS_NBLOCKS; ++i) {
		fs->fat[i] = MAGIC_BLOCK_FREE;
	}

	release_fat_lock();
	release_inodes_lock();
	release_dir_lock();
	release_stats_lock();
	return 0;
}
