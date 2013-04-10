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

#include "shm_loader.h"
#include "options.h"

#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define SHM_PATH "/dev/shm/"
#define BS_SUFFIX "/data/"

/* POSIX shared memory is not required to be implemented as a regular filesystem,
 * however Linux does this, providing with a very convenient way to load files
 * into shared memory
 */
static bool copy_into_shm(char *src_name) {
	pid_t child = fork();
	if(child == -1) {
		perror("fork");
		return false;
	}

	if(child == 0) {
		execl("/bin/cp", "/bin/cp", src_name, SHM_PATH, (char *)NULL);
		return false;
	} else {
		int status = 0;
		waitpid(child, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
}

void *shm_ref_loader(void *loader_work) {
	pthread_detach(pthread_self());
	struct ref_loader_work *w = (struct ref_loader_work *)loader_work;
	// form fully qualified pathname
	char buf[PATH_MAX];
	snprintf(buf, sizeof(buf), "%s%s%s", shmdopts.bs_path, BS_SUFFIX, w->refname);
	// TODO: stat
	// exists) copy_into_shm
	if(copy_into_shm(buf)) {
		fprintf(stderr, "[RefLd] Loaded %s from local blockstore\n", w->refname);
		fflush(stderr);
	}
	// TODO: notfound) broadcast to other shmds
	// TODO: notify requester of completion
	free(loader_work);
	pthread_exit(NULL);
}
