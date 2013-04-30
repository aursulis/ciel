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

// This file will be compiled into a shared object

#include "ipc_defs.h"

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

static char shmd_socket_file[PATH_MAX];

static const char *get_client_dom_addr()
{
	static char template[] = CLIENT_SOCK_TEMPL;
	static char dom_addr[] = CLIENT_SOCK_TEMPL "/" SOCKET_FILE;
	static bool is_set = false;

	if(!is_set) {
		mkdtemp(template);
		snprintf(dom_addr, sizeof(dom_addr), "%s/" SOCKET_FILE, template);
		is_set = true;
	}
	
	return dom_addr;
}

void ipc_init_client(const char *bs_path)
{
	snprintf(shmd_socket_file, sizeof(shmd_socket_file), "%s/../" SOCKET_FILE, bs_path);
}

static int ipc_do_request(const struct ipc_request *rq, struct ipc_response *rsp) // XXX: sock_fd is not being closed on errors
{
	// TODO: if socket creation becomes a bottleneck, consider having
	// a singleton socket that is reused (and mutex protected)

	int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(sock_fd == -1) {
		perror("socket");
		return -1;
	}

	struct sockaddr_un claddr;
	memset(&claddr, 0, sizeof(struct sockaddr_un));
	claddr.sun_family = AF_UNIX;
	strncpy(claddr.sun_path, get_client_dom_addr(), sizeof(claddr.sun_path));

	if(bind(sock_fd, (struct sockaddr *)&claddr, sizeof(claddr)) == -1) {
		perror("bind");
		return -1;
	}

	struct sockaddr_un servaddr;
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strncpy(servaddr.sun_path, shmd_socket_file, sizeof(servaddr.sun_path));

	if(connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("connect");
		return -1;
	}

	ssize_t bytes = send(sock_fd, rq, sizeof(*rq), 0);
	if(bytes == -1) {
		perror("send");
		return -1;
	}

	socklen_t servlen;
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	bytes = recvfrom(sock_fd, rsp, sizeof(*rsp), 0,
			(struct sockaddr *)&servaddr, &servlen);

	close(sock_fd);
	return 0;
}

int ipc_send_load_request(const char *refname, char *shmname)
{
	struct ipc_request rq;
	rq.header.len = sizeof(rq);
	rq.header.type = IPC_REQ_LD;
	strncpy(rq.refname, refname, sizeof(rq.refname));

	struct ipc_response rsp;

	int rc = ipc_do_request(&rq, &rsp);

	if(rsp.header.type == IPC_RSP_OK) {
		strncpy(shmname, rsp.shmname, sizeof(rsp.shmname));
		return 0;
	} else if(rsp.header.type == IPC_RSP_FAIL) {
		return -1;
	}
}

int ipc_send_write_request(const char *refname, char *shmname)
{
	struct ipc_request rq;
	rq.header.len = sizeof(rq);
	rq.header.type = IPC_REQ_WR;
	strncpy(rq.refname, refname, sizeof(rq.refname));

	struct ipc_response rsp;

	int rc = ipc_do_request(&rq, &rsp);

	if(rsp.header.type == IPC_RSP_OK) {
		strncpy(shmname, rsp.shmname, sizeof(rsp.shmname));
		return 0;
	} else if(rsp.header.type == IPC_RSP_FAIL) {
		return -1;
	}
}
