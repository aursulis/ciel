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

#ifndef SHM_FS_ARCH_H
#define SHM_FS_ARCH_H

#include "shm_fs.h"

void shmfs_control_init(int id);
struct shmfs *shmfs_data_init(int id);

void get_stats_lock();
void get_dir_lock();
void get_inodes_lock();
void get_fat_lock();

void release_stats_lock();
void release_dir_lock();
void release_inodes_lock();
void release_fat_lock();

void wait_barrier();

#endif
