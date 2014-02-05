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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_TEMPLATE "/tmp/intershmd/shmd-"
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

void *interdaemon_server_main(void *ignored)
{
	log_f("IdSrv", "Interdaemon server thread started\n");

	if(interdaemon_create_pipe() == -1) pthread_exit(NULL);
	int pipe_fd = interdaemon_get_read_pipe();

	int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(sock_fd == -1) {
		perror("socket");
		pthread_exit(NULL);
	}

	struct sockaddr_un servaddr;
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	snprintf(servaddr.sun_path, sizeof(servaddr.sun_path), SOCKET_TEMPLATE "%d", shmdopts.shmd_id);

	if(bind(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind");
		pthread_exit(NULL);
	}

	struct sockaddr_un srcaddr;
	socklen_t srclen = sizeof(srcaddr);

	fd_set reference_set;
	FD_ZERO(&reference_set);
	FD_SET(pipe_fd, &reference_set);
	FD_SET(sock_fd, &reference_set);
	int nfds = MAX(pipe_fd, sock_fd) + 1;

	fd_set work_set;
	while(1) {
		log_f("IdSrv", "waiting for incoming message or queued reply\n");

		work_set = reference_set;
		int rc = select(nfds, &work_set, NULL, NULL, NULL);
		if(rc == -1) {
			perror("select");
			pthread_exit(NULL);
		}

		if(FD_ISSET(sock_fd, &work_set)) {
			char buf[sizeof(struct ipc_request)];
			ssize_t bytes = recvfrom(sock_fd, buf, sizeof(buf), 0,
					(struct sockaddr *)&srcaddr, &srclen);

			interdaemon_handle_external(buf, &srcaddr, srclen, -1); // don't use from_shmd
		}

		if(FD_ISSET(pipe_fd, &work_set)) {
			struct shm_worker_w *w = NULL;
			ssize_t bytes = read(pipe_fd, &w, sizeof(w));

			if(w->stage == STAGE_RECURSIVE_RQ) {
				log_f("IdSrv", "received recursive request from shm_worker for %s\n", w->rq.refname);
				for(int i = 0; i < shmdopts.nshmds; ++i) {
					if(i == shmdopts.shmd_id) continue;

					struct sockaddr_un dstaddr;
					memset(&dstaddr, 0, sizeof(struct sockaddr_un));
					dstaddr.sun_family = AF_UNIX;
					snprintf(dstaddr.sun_path, sizeof(dstaddr.sun_path), SOCKET_TEMPLATE "%d", i);

					log_f("IdSrv", "sending recursive request to %d\n", i);

					sendto(sock_fd, &w->rq, sizeof(w->rq), 0,
							(struct sockaddr *)&dstaddr, sizeof(dstaddr));
				}
			} else if(w->stage == STAGE_RSP) {
				log_f("IdSrv", "received response from shm_worker\n");

				ssize_t send_bytes = sendto(sock_fd, &w->rsp, sizeof(w->rsp), 0,
						(struct sockaddr *)&w->replyaddr, w->replylen);

				free(w);
			}
		}
	}
	pthread_exit(NULL);
}
