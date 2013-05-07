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
#include "shm_fs_arch.h"

#ifdef BUILD_LINUX
	#include "shm_fs_linux.h"
#else
	#error "Please implement me"
#endif

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static struct shmfs *fs = NULL;

void shmfs_init(int id)
{
	shmfs_control_init(id);
	fs = shmfs_data_init(id);

	if(id == 0) {
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

	// XXX: BARRIER HERE
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

static inline int find_next_free_block(int from)
{
	for(int k = 0; k < SHMFS_NBLOCKS; ++k) {
		if(fs->fat[(from+k) % SHMFS_NBLOCKS] == MAGIC_BLOCK_FREE)
			return (from+k) % SHMFS_NBLOCKS;
	}
	return MAGIC_INVALID_ENTRY;
}

int shmfs_create(const char *name, bool openwrite)
{
	int rc = MAGIC_INVALID_ENTRY;
	
	get_dir_lock();

	for(int i = 0; i < SHMFS_NFILES; ++i) {
		if(fs->files[i].inode_id == MAGIC_INVALID_ENTRY) {
			get_inodes_lock();

			for(int j = 0; j < SHMFS_NINODES; ++j) {
				if(fs->inodes[j].flags.valid == 0) {
					get_fat_lock();

					int k = find_next_free_block(0);
					if(k != MAGIC_INVALID_ENTRY) {
						fs->fat[k] = MAGIC_BLOCK_LAST;

						fs->inodes[j].nlinks = 1;
						fs->inodes[j].nopen = openwrite;
						fs->inodes[j].flags.valid = 1;
						fs->inodes[j].flags.openwrite = openwrite;
						fs->inodes[j].flags.committed = 0;
						fs->inodes[j].first_block = k;
						fs->inodes[j].size = 0;

						rc = j;
						fs->files[i].inode_id = j;
						strncpy(fs->files[i].name, name, sizeof(fs->files[i].name));
					}

					release_fat_lock();
					break;
				}
			}

			release_inodes_lock();
			break;
		}
	}

	release_dir_lock();

	if(rc != MAGIC_INVALID_ENTRY) {
		get_stats_lock();
		fs->stats.free_dirents--;
		fs->stats.free_inodes--;
		fs->stats.free_blocks--;
		if(openwrite) fs->stats.nwrites++;
		release_stats_lock();
	}

	return rc;
}

int shmfs_link(const char *target, const char *name)
{
	return 0;
}

int shmfs_load_local(const char *name)
{
	FILE *f_src = fopen(name, "rb");
	int inode_id = shmfs_create(basename(name), true);

	get_fat_lock();

	int cur_block = fs->inodes[inode_id].first_block;
	size_t bytes = 0;
	size_t total_size = 0;
	int blocks_reserved = 0;

	do {
		bytes = fread(fs->blocks[cur_block].d, sizeof(char),
				SHMFS_BSIZE, f_src);

		total_size += bytes;
		if(bytes == SHMFS_BSIZE) {
			int next_block = find_next_free_block(cur_block);
			assert(next_block != MAGIC_INVALID_ENTRY);
			if(next_block != MAGIC_INVALID_ENTRY) {
				fs->fat[cur_block] = next_block;
				fs->fat[next_block] = MAGIC_BLOCK_LAST;
				cur_block = next_block;
				blocks_reserved++;
			}
		}
	} while(bytes > 0);

	release_fat_lock();
	fclose(f_src);

	get_stats_lock();
	get_inodes_lock();
	fs->stats.free_blocks -= blocks_reserved;
	fs->stats.nwrites--;

	fs->inodes[inode_id].size = total_size;
	fs->inodes[inode_id].nopen = 0;
	fs->inodes[inode_id].flags.openwrite = 0;
	fs->inodes[inode_id].flags.committed = 1;
	release_inodes_lock();
	release_stats_lock();

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
