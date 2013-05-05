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

#include "logging.h"
#include "shm_worker.h"
#include "options.h"

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define SHM_PATH "/dev/shm/"
#define BS_SUFFIX "/data/"

/* POSIX shared memory is not required to be implemented as a regular filesystem,
 * however Linux does this, providing with a very convenient way to load files
 * into shared memory
 */
static bool copy_into_shm(char *src_name)
{
	pid_t child = fork();
	if(child == -1) {
		perror("fork");
		return false;
	}

	if(child == 0) {
		execl("/bin/cp", "/bin/cp", src_name, SHM_PATH, (char *)NULL);
		return false;
	} else {
		int status = 0;
		waitpid(child, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
}

static void get_shm_name(const char *refname, char *shmname)
{
	snprintf(shmname, PATH_MAX, SHM_PATH "%s", basename(refname));
}

void *shm_worker(void *work)
{
	pthread_detach(pthread_self());
	struct shm_worker_w *w = (struct shm_worker_w *)work;

	log_f("ShmWrk", "Worker thread started for %s\n", w->rq.refname);

	if(w->rq.header.type == IPC_REQ_LD) {
		log_f("ShmWrk", "Load requested for %s\n", w->rq.refname);

		struct stat st;
		int rc = stat(w->rq.refname, &st);

		if(rc == 0) {
			// TODO: check memory availability

			// exists) copy_into_shm
			if(copy_into_shm(w->rq.refname)) {
				log_f("ShmWrk", "Loaded %s from local blockstore\n", w->rq.refname);
				get_shm_name(w->rq.refname, w->rsp.shmname);
				w->rsp.header.type = IPC_RSP_OK;
			} else {
				w->rsp.header.type = IPC_RSP_FAIL;
			}
		} else {
			if(errno == ENOENT) {
				// TODO: notfound) broadcast to other shmds
				log_f("ShmWrk", "%s not found locally\n", w->rq.refname);
			} else {
				perror("stat");
				w->rsp.header.type = IPC_RSP_FAIL;
			}
		}
	} else if(w->rq.header.type == IPC_REQ_WR) {
		log_f("ShmWrk", "Write requested for %s\n", w->rq.refname);
		get_shm_name(w->rq.refname, w->rsp.shmname);
		w->rsp.header.type = IPC_RSP_OK;
	}

	// notify requester of completion
	write(w->replyfd, &w, sizeof(w));
	pthread_exit(NULL);
}
