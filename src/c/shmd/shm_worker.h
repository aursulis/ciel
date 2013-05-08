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

#ifndef SHM_WORKER_H
#define SHM_WORKER_H

#include "ipc_defs.h"

#include <limits.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>

enum shm_worker_stage_t { STAGE_RQ, STAGE_RECURSIVE_RQ, STAGE_RSP };

struct shm_worker_w
{
	struct ipc_request rq;
	struct ipc_response rsp;
	int replyfd;
	struct sockaddr_un replyaddr;
	socklen_t replylen;
	int replyshmd;
	bool recursive;
	enum shm_worker_stage_t stage;
};

void *shm_worker(void *work);

#endif
