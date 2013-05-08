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

#include "interdaemon.h"
#include "logging.h"
#include "ipc_defs.h"
#include "shm_worker.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

static int pipe_fd[2];

int interdaemon_get_read_pipe()
{
	return pipe_fd[0];
}

int interdaemon_get_write_pipe()
{
	return pipe_fd[1];
}

int interdaemon_create_pipe()
{
	if(pipe(pipe_fd) == -1) {
		perror("pipe");
		return -1;
	}
	return 0;
}

void interdaemon_handle_external(char *buf, struct sockaddr_un *srcaddr, socklen_t srclen, int from_shmd)
{
	struct ipc_header *h = (struct ipc_header *)buf;
	if(h->type == IPC_REQ_LD) {
		struct ipc_request *rq = (struct ipc_request *)buf;
		log_f("IdSrv", "received external request to load %s\n", rq->refname);

		struct shm_worker_w *w = (struct shm_worker_w *)malloc(sizeof(struct shm_worker_w));
		w->rq = *rq;
		w->rsp.header.replyfd = w->rq.header.replyfd;
		w->rsp.header.len = sizeof(w->rsp);
		w->replyaddr = *srcaddr;
		w->replylen = srclen;
		w->replyfd = pipe_fd[1];
		w->replyshmd = from_shmd;
		w->recursive = false;
		w->stage = STAGE_RQ;

		pthread_t worker_thread;
		pthread_create(&worker_thread, NULL, shm_worker, (void *)w);
	} else if(h->type == IPC_RSP_OK || h->type == IPC_RSP_FAIL) {
		struct ipc_response *rsp = (struct ipc_response *)buf;
		log_f("IdSrv", "received external response\n");

		write(rsp->header.replyfd, rsp, sizeof(*rsp));
	}
}
