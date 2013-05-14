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

#ifndef SHM_FS_SCC_H
#define SHM_FS_SCC_H

#include "shm_fs.h"
#include "shm_fs_arch.h"

#include "RCCE.h"

void shmfs_control_init(int id) { }

inline void get_stats_lock() { RCCE_acquire_lock(0); }
inline void get_dir_lock() { RCCE_acquire_lock(1); }
inline void get_inodes_lock() { RCCE_acquire_lock(2); }
inline void get_fat_lock() { RCCE_acquire_lock(3); }

inline void release_stats_lock() { RCCE_release_lock(0); }
inline void release_dir_lock() { RCCE_release_lock(1); }
inline void release_inodes_lock() { RCCE_release_lock(2); }
inline void release_fat_lock() { RCCE_release_lock(3); }

inline void wait_barrier() { RCCE_barrier(&RCCE_COMM_WORLD); }

#define SHMOFFSET (256*1024*1024)
struct shmfs *shmfs_data_init(int id)
{
	struct shmfs *result;
	result = (struct shmfs *)(RCCE_shmalloc(sizeof(*result)) + SHMOFFSET);
	return result;
}

#endif
