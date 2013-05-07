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

#ifndef SHM_WORKER_LINUX_H
#define SHM_WORKER_LINUX_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define SHM_PATH "/dev/shm/"

/* POSIX shared memory is not required to be implemented as a regular filesystem,
 * however Linux does this, providing with a very convenient way to load files
 * into shared memory
 */
static bool copy_into_shm(char *src_name)
{
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

static bool invoke_ln(char *src_name, char *dst_name)
{
	pid_t child = fork();
	if(child == -1) {
		perror("fork");
		return false;
	}

	if(child == 0) {
		execl("/bin/ln", "/bin/ln", src_name, dst_name, (char *)NULL);
		return false;
	} else {
		int status = 0;
		waitpid(child, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
}

static void get_shm_name(const char *refname, char *shmname)
{
	snprintf(shmname, PATH_MAX, SHM_PATH "%s", basename(refname));
}

static bool is_present_in_shmfs(const char *refname)
{
	char buf[PATH_MAX];
	get_shm_name(refname, buf);

	struct stat st;
	int rc = stat(buf, &st);

	return rc == 0;
}

static void copy_shmname(const char *refname, char *shmname)
{
	get_shm_name(refname, shmname);
}

#endif
