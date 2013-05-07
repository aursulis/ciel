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

#ifndef SHM_FS_H
#define SHM_FS_H

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

#define SHMFS_NFILES   128   // number of directory entries
#define SHMFS_NINODES  128   // number of inodes
#define SHMFS_BSIZE    (1024*1024)   // block size: 1 MB
#define SHMFS_NBLOCKS  256   // number of blocks

#define MAGIC_BLOCK_FREE (-1) // block is free
#define MAGIC_BLOCK_LAST (-2) // block contains EOF
#define MAGIC_INVALID_ENTRY (-1) // directory entry not used

struct directory_entry
{
	char name[NAME_MAX];
	int inode_id;
};

struct inode
{
	int nlinks; // number of directory entries for this inode
	int nopen; // number of fifos that are serving this inode
	struct {
		unsigned int valid     : 1; // is this a valid (existent) inode?
		unsigned int openwrite : 1; // is this inode open for writing?
		unsigned int committed : 1; // have writes to this inode been committed?
	} flags;
	int first_block;
	size_t size; // in bytes, not blocks
};

struct block
{
	char d[SHMFS_BSIZE];
};

struct statistics
{
	int nwrites; // number of input fifos currently open
	int free_dirents;
	int free_inodes;
	int free_blocks;
};

struct shmfs
{
	struct statistics stats; // filesystem statistics
	struct directory_entry files[SHMFS_NFILES]; // directory
	struct inode inodes[SHMFS_NINODES]; // inode table
	int fat[SHMFS_NBLOCKS]; // file allocation table
	struct block blocks[SHMFS_NBLOCKS]; // direct access to data
};

void shmfs_init(int id);
int shmfs_lookup(const char *name);
int shmfs_create(const char *name, bool openwrite);
int shmfs_link(const char *target, const char *name);
int shmfs_commit(const char *oldname, const char *newname);
int shmfs_load_local(const char *name);
int shmfs_store_local(int inode_id, const char *name);
int shmfs_deallocate(int inode_id);
int shmfs_unlink(const char *name);
size_t shmfs_getsize(const char *name);

#endif
