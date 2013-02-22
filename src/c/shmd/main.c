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

#include "options.h"
#include "ipc_server.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define LOCK_FILE "shmd.pid"

static int get_lock_file(void)
{
	int lock_fd = open(LOCK_FILE, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	if(lock_fd == -1) {
		if(errno == EEXIST) {
			fprintf(stderr, "shmd already running\n");
			exit(EXIT_SUCCESS);
		} else {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}

	return lock_fd;
}

static void setup_log_file(time_t *t)
{
	char buf[32];
	strftime(buf, sizeof(buf), "shmd-%Y%m%d-%H%M%S.log", localtime(t));
	freopen(buf, "w", stderr);
}

static void write_pid(int fd)
{
	char buf[16];
	sprintf(buf, "%d\n", getpid());
	write(fd, buf, strlen(buf));
	fsync(fd);
}

int main(int argc, char **argv)
{
	parse_options(argc, argv, &shmdopts);

	chdir(shmdopts.bs_path);

	int lock_fd = get_lock_file();
	write_pid(lock_fd);
	close(lock_fd);

	time_t start_time = time(NULL);

	if(shmdopts.daemonise) {
		if(daemon(1, 0) == -1) {
			perror("daemon");
			exit(EXIT_FAILURE);
		}

		setup_log_file(&start_time);
	}

	char buf[64];
	strftime(buf, sizeof(buf), "Started shmd on %Y-%m-%d %H:%M:%S", localtime(&start_time));
	fprintf(stderr, "%s\n", buf);

	pthread_t ipc_thread;
	pthread_create(&ipc_thread, NULL, ipc_server_main, NULL);

	pthread_join(ipc_thread, NULL);

	unlink(LOCK_FILE);
	return EXIT_SUCCESS;
}
