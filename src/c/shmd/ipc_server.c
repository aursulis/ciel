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
#include "ipc_server.h"
#include "ipc_defs.h"
#include "shm_loader.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

void *ipc_server_main(void *ignored)
{
	log_f("IpcSrv", "IPC server thread started\n");

	int pipe_fd[2];
	if(pipe(pipe_fd) == -1) {
		perror("pipe");
		pthread_exit(NULL);
	}

	int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(sock_fd == -1) {
		perror("socket");
		pthread_exit(NULL);
	}

	struct sockaddr_un servaddr;
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strncpy(servaddr.sun_path, SOCKET_FILE, sizeof(servaddr.sun_path));

	if(bind(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind");
		pthread_exit(NULL);
	}

	struct sockaddr_un srcaddr;
	socklen_t srclen = sizeof(srcaddr);

	fd_set reference_set;
	FD_ZERO(&reference_set);
	FD_SET(pipe_fd[0], &reference_set);
	FD_SET(sock_fd, &reference_set);
	int nfds = MAX(pipe_fd[0], sock_fd) + 1;

	fd_set work_set;
	while(1) {
		log_f("IpcSrv", "waiting for incoming message or queued reply\n");

		work_set = reference_set;
		int rc = select(nfds, &work_set, NULL, NULL, NULL);
		if(rc == -1) {
			perror("select");
			pthread_exit(NULL);
		}

		if(FD_ISSET(sock_fd, &work_set)) {
			struct ipc_request rq;
			ssize_t bytes = recvfrom(sock_fd, &rq, sizeof(rq), 0,
					(struct sockaddr *)&srcaddr, &srclen);

			if(rq.header.type == IPC_REQ_LD) {
				log_f("IpcSrv", "received request to load %s\n", rq.refname);

				struct ref_loader_work *w = (struct ref_loader_work *)malloc(sizeof(struct ref_loader_work));
				strncpy(w->refname, rq.refname, sizeof(w->refname));
				w->replyaddr = srcaddr;
				w->replylen = srclen;
				w->replyfd = pipe_fd[1];

				pthread_t loader_thread;
				pthread_create(&loader_thread, NULL, shm_ref_loader, (void *)w);
			} else if(rq.header.type == IPC_REQ_WR) {
				log_f("IpcSrv", "received request to write %s\n", rq.refname);

				// TODO: do something
			}
		}

		if(FD_ISSET(pipe_fd[0], &work_set)) {
			struct ref_loader_work *w = NULL;
			ssize_t bytes = read(pipe_fd[0], &w, sizeof(w));
			
			struct ipc_response rsp;
			rsp.header.len = sizeof(rsp);

			if(w->status == LOAD_SUCCESS) {
				rsp.header.type = IPC_RSP_OK;
				strncpy(rsp.shmname, w->loadedname, sizeof(rsp.shmname));
			} else if(w->status == LOAD_FAIL) {
				rsp.header.type = IPC_RSP_FAIL;
			}

			ssize_t send_bytes = sendto(sock_fd, &rsp, sizeof(rsp), 0,
					(struct sockaddr *)&w->replyaddr, w->replylen);

			free(w);
		}
	}
}
