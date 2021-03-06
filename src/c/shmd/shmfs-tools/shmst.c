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

#include "shm_fs.h"

#ifndef KERN_LINUX
	#error "Available only on normal Linux"
#endif

#include <string.h>

int main(int argc, char **argv)
{
	shmfs_init(1);
	int inode_id = shmfs_lookup(basename(argv[1]));
	if(inode_id == MAGIC_INVALID_ENTRY) return -1;

	shmfs_store_local(inode_id, argv[2]);
	return 0;
}
