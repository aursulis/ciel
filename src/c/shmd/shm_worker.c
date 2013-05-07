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
#include "interdaemon.h"
#include "options.h"

#if defined(BUILD_POSIX_SHMFS)
	#include "shm_worker_linux.h"
#elif defined(BUILD_CUSTOM_SHMFS)
	#error "Implement me"
#endif

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>

#define BS_SUFFIX "/data/"

static void normalise_refname(char *refname)
{
	char buf[PATH_MAX];
	snprintf(buf, sizeof(buf), "%s" BS_SUFFIX "%s", shmdopts.bs_path, basename(refname));
	strncpy(refname, buf, PATH_MAX);
}

void *shm_worker(void *work)
{
	pthread_detach(pthread_self());
	struct shm_worker_w *w = (struct shm_worker_w *)work;

	log_f("ShmWrk", "Worker thread started for %s\n", w->rq.refname);

	normalise_refname(w->rq.refname);

	if(w->rq.header.type == IPC_REQ_LD) {
		log_f("ShmWrk", "Load requested for %s\n", w->rq.refname);

		bool present = is_present_in_shmfs(w->rq.refname);

		if(present) { // already present in shm
			copy_shmname(w->rq.refname, w->rsp.shmname);
			w->rsp.header.type = IPC_RSP_OK;
			w->stage = STAGE_RSP;
		} else {
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

				w->stage = STAGE_RSP;
			} else {
				if(errno == ENOENT) {
					log_f("ShmWrk", "%s not found locally\n", w->rq.refname);
					if(w->recursive) {
						int pipe_fd[2];
						if(pipe(pipe_fd) == -1) {
							perror("pipe");
							pthread_exit(NULL);
						}

						w->stage = STAGE_RECURSIVE_RQ;
						w->rq.header.replyfd = pipe_fd[1];
						w->rsp.header.type = IPC_RSP_FAIL; // fail by default unless some recursive request succeeds

						log_f("ShmWrk", "making recursive call for %s\n", w->rq.refname);
						write(interdaemon_get_pipe(), &w, sizeof(w));

						int nresponses = 0;
						while(nresponses < shmdopts.nshmds-1) {
							struct ipc_response rsp;
							read(pipe_fd[0], &rsp, sizeof(rsp));

							log_f("ShmWrk", "got back recursive response for %s\n", w->rq.refname);
							if(rsp.header.type == IPC_RSP_OK) {
								w->rsp = rsp;
								// TODO: on SCC, open the file and adjust filename
							}

							++nresponses;
						}

						close(pipe_fd[0]);
						close(pipe_fd[1]);
					} else {
						log_f("ShmWrk", "nonrecursive call for %s, failing\n", w->rq.refname);
						w->rsp.header.type = IPC_RSP_FAIL;
					}
				} else {
					perror("stat");
					w->rsp.header.type = IPC_RSP_FAIL;
				}

				w->stage = STAGE_RSP;
			}
		}
	} else if(w->rq.header.type == IPC_REQ_WR) {
		log_f("ShmWrk", "Write requested for %s\n", w->rq.refname);
		get_shm_name(w->rq.refname, w->rsp.shmname);
		w->rsp.header.type = IPC_RSP_OK;
		w->stage = STAGE_RSP;
	} else if(w->rq.header.type == IPC_REQ_CI) {
		log_f("ShmWrk", "Commit requested for %s as %s\n", w->rq.refname, w->rsp.shmname);

		char oldname[PATH_MAX], newname[PATH_MAX];
		get_shm_name(w->rq.refname, oldname);
		get_shm_name(w->rsp.shmname, newname);

		invoke_ln(oldname, newname);

		w->rsp.header.type = IPC_RSP_OK;
		w->stage = STAGE_RSP;
	}

	// notify requester of completion
	write(w->replyfd, &w, sizeof(w));
	pthread_exit(NULL);
}
