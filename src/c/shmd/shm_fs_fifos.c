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
#include "shm_fs_helpers.h"
#include "shm_fs_arch.h"
#include "shm_fs_fifos.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NO_FIFO (-1)

static int fifo_dir_id = 0;
static int writer_fifos[SHMFS_NINODES];
static int fifo_counter = 0;
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fifos_lock = PTHREAD_MUTEX_INITIALIZER;

void shmfs_fifos_init(int id)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "/tmp/shmfs-fifos-%d/", id);
	mkdir(buf, 00755);
	fifo_dir_id = id;
}

static int new_fifo_number()
{
	pthread_mutex_lock(&counter_lock);

	fifo_counter++;
	if(fifo_counter < 0) fifo_counter = 0;

	int result = fifo_counter;
	pthread_mutex_unlock(&counter_lock);
	return result;
}

static void set_writer_fifo(int inode_id, int fifo_id)
{
	pthread_mutex_lock(&fifos_lock);
	writer_fifos[inode_id] = fifo_id;
	pthread_mutex_unlock(&fifos_lock);
}

static int get_writer_fifo(int inode_id)
{
	pthread_mutex_lock(&fifos_lock);
	int result = writer_fifos[inode_id];
	pthread_mutex_unlock(&fifos_lock);
	return result;
}

void *shmfs_fifo_input(void *args)
{
	pthread_detach(pthread_self());

	struct shmfs_fifo_w *w = (struct shmfs_fifo_w *)args;

	int inode_id = w->inode_id;
	int blocks_reserved = 0;
	size_t total_size = 0;

	while(total_size == 0) { // Hack to get around CIEL's open-and-close-immediately behaviour
		FILE *f_in = fopen(w->fifoname, "rb");
		total_size = perform_input_loop(f_in, inode_id, &blocks_reserved);
		fclose(f_in);
	}

	get_stats_lock();
	get_inodes_lock();
	fs->stats.free_blocks -= blocks_reserved;

	fs->inodes[inode_id].size = total_size;
	fs->inodes[inode_id].nopen = 0;
	fs->inodes[inode_id].flags.openwrite = 0;
	fs->inodes[inode_id].flags.committed = 0;
	release_inodes_lock();
	release_stats_lock();

	set_writer_fifo(inode_id, NO_FIFO);
	unlink(w->fifoname);
	free(w);
	pthread_exit(NULL);
}

void *shmfs_fifo_output(void *args)
{
	pthread_detach(pthread_self());

	struct shmfs_fifo_w *w = (struct shmfs_fifo_w *)args;
	int inode_id = w->inode_id;

	FILE *f_out = fopen(w->fifoname, "wb");
	perform_output_loop(f_out, inode_id);
	fclose(f_out);

	get_inodes_lock();
	fs->inodes[inode_id].nopen--;
	release_inodes_lock();

	unlink(w->fifoname);
	free(w);
	pthread_exit(NULL);
}

static void form_fifo_name(char *dst, int fifo_id)
{
	snprintf(dst, PATH_MAX, "/tmp/shmfs-fifos-%d/%d", fifo_dir_id, fifo_id);
}

int shmfs_get_read_filename(const char *name, char *shmname)
{
	int inode_id = shmfs_lookup(name);
	if(inode_id == MAGIC_INVALID_ENTRY) {
		return -1;
	}

	int fifo_id = new_fifo_number();
	struct shmfs_fifo_w *w = (struct shmfs_fifo_w *)malloc(sizeof(struct shmfs_fifo_w));
	w->inode_id = inode_id;
	form_fifo_name(w->fifoname, fifo_id);
	mkfifo(w->fifoname, S_IRUSR | S_IWUSR);

	get_inodes_lock();
	fs->inodes[inode_id].nopen++;
	release_inodes_lock();

	pthread_t fifo_thread;
	pthread_create(&fifo_thread, NULL, shmfs_fifo_output, (void *)w);

	form_fifo_name(shmname, fifo_id);
	return 0;
}

int shmfs_get_write_filename(const char *name, char *shmname)
{
	int inode_id = shmfs_lookup(name);
	int fifo_id;

	if(inode_id == MAGIC_INVALID_ENTRY) {
		inode_id = shmfs_create(name, true);
		if(inode_id == MAGIC_INVALID_ENTRY) {
			return -1;
		}

		fifo_id = new_fifo_number();
		struct shmfs_fifo_w *w = (struct shmfs_fifo_w *)malloc(sizeof(struct shmfs_fifo_w));
		w->inode_id = inode_id;
		form_fifo_name(w->fifoname, fifo_id);
		mkfifo(w->fifoname, S_IRUSR | S_IWUSR);

		pthread_t fifo_thread;
		pthread_create(&fifo_thread, NULL, shmfs_fifo_input, (void *)w);
		set_writer_fifo(inode_id, fifo_id);
	} else {
		fifo_id = get_writer_fifo(inode_id);
		// XXX: we assume that all files opened for writing were created through this
		// function; otherwise the above call returns garbage
	}

	form_fifo_name(shmname, fifo_id);
	return 0;
}
