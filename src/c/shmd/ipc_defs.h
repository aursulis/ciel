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

#ifndef IPC_DEFS_H
#define IPC_DEFS_H

#include <limits.h>
#include <stddef.h>

#define CLIENT_SOCK_TEMPL "/tmp/shmdc-XXXXXX"
#define SOCKET_FILE "shmd.sock"

enum ipc_message_t { IPC_REQ_LD, IPC_REQ_WR, IPC_RSP_OK, IPC_RSP_FAIL };

struct ipc_header
{
	size_t len;
	enum ipc_message_t type;
};

struct ipc_request
{
	struct ipc_header header;
	char refname[PATH_MAX];
};

struct ipc_response
{
	struct ipc_header header;
	char shmname[PATH_MAX];
};

#endif
