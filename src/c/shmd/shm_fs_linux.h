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

#ifndef SHM_FS_LINUX_H
#define SHM_FS_LINUX_H

#include "shm_fs_control.h"
#include <pthread.h>

static struct shmfs_control
{
	pthread_mutex_t stats_lock;
	pthread_mutex_t dir_lock;
	pthread_mutex_t inodes_lock;
	pthread_mutex_t fat_lock;
} *c = NULL;

void shmfs_control_init()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

	pthread_mutex_init(&c->stats_lock, &attr);
	pthread_mutex_init(&c->dir_lock, &attr);
	pthread_mutex_init(&c->inodes_lock, &attr);
	pthread_mutex_init(&c->fat_lock, &attr);

	pthread_mutexattr_destroy(&attr);
}

inline void get_stats_lock() { pthread_mutex_lock(&c->stats_lock); }
inline void get_dir_lock() { pthread_mutex_lock(&c->dir_lock); }
inline void get_inodes_lock() { pthread_mutex_lock(&c->inodes_lock); }
inline void get_fat_lock() { pthread_mutex_lock(&c->fat_lock); }

inline void release_stats_lock() { pthread_mutex_unlock(&c->stats_lock); }
inline void release_dir_lock() { pthread_mutex_unlock(&c->dir_lock); }
inline void release_inodes_lock() { pthread_mutex_unlock(&c->inodes_lock); }
inline void release_fat_lock() { pthread_mutex_unlock(&c->fat_lock); }

#endif
