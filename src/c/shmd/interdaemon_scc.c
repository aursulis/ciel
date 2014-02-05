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

#include "logging.h"
#include "options.h"
#include "interdaemon.h"
#include "interdaemon_arch.h"
#include "ipc_defs.h"
#include "shm_worker.h"

#include "iRCCE.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

void *interdaemon_server_main(void *ignored)
{
	log_f("IdSrv", "Interdaemon server thread started\n");

	if(interdaemon_create_pipe() == -1) pthread_exit(NULL);
	int pipe_fd = interdaemon_get_read_pipe();
	int flags = fcntl(pipe_fd, F_GETFL);
	fcntl(pipe_fd, F_SETFL, flags | O_NONBLOCK);

	char buf[sizeof(struct ipc_request)];
	struct shm_worker_w *w = NULL;

	iRCCE_RECV_REQUEST r_rq;
	iRCCE_SEND_REQUEST s_rq_bcast[shmdopts.nshmds];
	iRCCE_SEND_REQUEST s_rq_reply;

	bool init_recv = true;
	bool init_send = true;
	bool bcast_pending = false;
	bool reply_pending = false;

	while(1) {
		//log_f("IdSrv", "waiting for incoming message or queued reply\n");

		if(init_recv) {
			iRCCE_irecv(buf, sizeof(buf), iRCCE_ANY_SOURCE, &r_rq);
			init_recv = false;
		}

		if(init_send) {
			ssize_t bytes = read(pipe_fd, &w, sizeof(w));

			if(bytes < 0 && errno != EAGAIN) {
				perror("read");
				pthread_exit(NULL);
			} else if(bytes >= 0) {
				if(w->stage == STAGE_RECURSIVE_RQ) {
					log_f("IdSrv", "received recursive request from shm_worker for %s\n", w->rq.refname);

					for(int i = 0; i < shmdopts.nshmds; ++i) {
						if(i == shmdopts.shmd_id) continue;

						log_f("IdSrv", "sending recursive request to %d\n", i);
						iRCCE_isend((char *)&w->rq, sizeof(w->rq), i, &s_rq_bcast[i]);
					}

					bcast_pending = true;
				} else if(w->stage == STAGE_RSP) {
					log_f("IdSrv", "received response from shm_worker\n");

					iRCCE_isend((char *)&w->rsp, sizeof(w->rsp), w->replyshmd, &s_rq_reply);

					reply_pending = true;
				}

				init_send = false;
			}
		}

		if(!init_recv) {
			if(iRCCE_irecv_test(&r_rq, NULL) == iRCCE_SUCCESS) {
				int from_shmd = iRCCE_get_source(&r_rq);

				struct sockaddr_un srcaddr; // dummy values for handle_external
				socklen_t srclen = sizeof(srcaddr);

				interdaemon_handle_external(buf, &srcaddr, srclen, from_shmd);

				init_recv = true;
			}
		}

		if(bcast_pending) {
			int collected_success = 0;
			for(int i = 0; i < shmdopts.nshmds; ++i) {
				if(i == shmdopts.shmd_id) continue;
				if(iRCCE_isend_test(&s_rq_bcast[i], NULL) == iRCCE_SUCCESS) collected_success++;
			}
			if(collected_success == shmdopts.nshmds-1) {
				bcast_pending = false;
				init_send = true;
			}
		}

		if(reply_pending) {
			if(iRCCE_isend_test(&s_rq_reply, NULL) == iRCCE_SUCCESS) {
				reply_pending = false;
				init_send = true;
				free(w);
			}
		}

		struct timespec tv;
		tv.tv_sec = 0;
		tv.tv_nsec = 100000000; // 0.1 s
		nanosleep(&tv, NULL); // don't hog the CPU
	}

	pthread_exit(NULL);
}
