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

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_FILE "shmd.sock"

void *ipc_server_main(void *ignored)
{
	fprintf(stderr, "[IpcSrv] IPC server thread started\n");

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

	char buf[1024];
	struct sockaddr_un srcaddr;
	socklen_t srclen = sizeof(srcaddr);
	while(1) {
		fprintf(stderr, "[IpcSrv] waiting for incoming message\n");

		ssize_t bytes = recvfrom(sock_fd, buf, sizeof(buf), 0,
				(struct sockaddr *)&srcaddr, &srclen);

		// cast buffer to appropriate struct,
		// perform operation
	}
}
