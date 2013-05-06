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

#include "shm_fs.h"
#include "shm_fs_control.h"

#ifdef BUILD_LINUX
	#include "shm_fs_linux.h"
#else
	#error "Please implement me"
#endif

#include <string.h>

static struct shmfs *fs = NULL;

void shmfs_setptr(char *fsptr)
{
	fs = (struct shmfs *)fsptr;
}

void shmfs_init()
{
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
}

int shmfs_lookup(const char *name)
{
	get_dir_lock();

	int result = MAGIC_INVALID_ENTRY;
	for(int i = 0; i < SHMFS_NFILES; ++i) {
		if(fs->files[i].inode_id != MAGIC_INVALID_ENTRY) {
			if(strncmp(name, fs->files[i].name, sizeof(fs->files[i].name)) == 0) {
				result = fs->files[i].inode_id;
				break;
			}
		}
	}

	release_dir_lock();

	return result;
}

int shmfs_create(const char *name)
{
	return 0;
}

int shmfs_link(const char *target, const char *name)
{
	return 0;
}

int shmfs_load_local(const char *name)
{
	return 0;
}

int shmfs_store_local(int inode_id)
{
	return 0;
}

int shmfs_deallocate(int inode_id)
{
	return 0;
}

int shmfs_unlink(const char *name)
{
	return 0;
}

size_t shmfs_getsize(const char *name)
{
	return 0;
}
