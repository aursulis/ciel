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

#ifndef SHM_FS_HELPERS_H
#define SHM_FS_HELPERS_H

#include "shm_fs.h"
#include "shm_fs_arch.h"

#include <assert.h>
#include <stdio.h>

extern struct shmfs *fs;

int find_next_free_block(int from);
size_t perform_input_loop(FILE *f_src, int inode_id, int *blocks_reserved);

inline int find_next_free_block(int from)
{
	for(int k = 0; k < SHMFS_NBLOCKS; ++k) {
		if(fs->fat[(from+k) % SHMFS_NBLOCKS] == MAGIC_BLOCK_FREE)
			return (from+k) % SHMFS_NBLOCKS;
	}
	return MAGIC_INVALID_ENTRY;
}

inline size_t perform_input_loop(FILE *f_src, int inode_id, int *blocks_reserved)
{
	int cur_block = fs->inodes[inode_id].first_block;
	size_t bytes = 0;
	size_t total_size = 0;
	*blocks_reserved = 0;

	do {
		bytes = fread(fs->blocks[cur_block].d, sizeof(char),
				SHMFS_BSIZE, f_src);

		total_size += bytes;
		if(bytes == SHMFS_BSIZE) {
			get_fat_lock();
			
			int next_block = find_next_free_block(cur_block);
			assert(next_block != MAGIC_INVALID_ENTRY);
			if(next_block != MAGIC_INVALID_ENTRY) {
				fs->fat[cur_block] = next_block;
				fs->fat[next_block] = MAGIC_BLOCK_LAST;
				cur_block = next_block;
				(*blocks_reserved)++;
			}

			release_fat_lock();
		}
	} while(bytes > 0);

	return total_size;
}

#endif
