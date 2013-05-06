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

#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

struct shmd_options shmdopts;

static void usage(const char *argv0)
{
	fprintf(stderr, "usage: %s OPTIONS\n", argv0);
	fprintf(stderr, "  -b PATH, --blockstore PATH : specify blockstore path\n");
	fprintf(stderr, "  -d,      --daemon          : run as daemon, log file in blockstore path\n");
	fprintf(stderr, "  -i NUM,  --id NUM          : specify ID of this shmd, must be unique\n");
	fprintf(stderr, "  -n NUM,  --numshmds NUM    : specify number of shmds running\n");
}

void parse_options(int argc, char **argv, struct shmd_options *opts)
{
	bool path_provided = false;
	opts->daemonise = false;
	opts->shmd_id = -1;
	opts->nshmds = 1;
	char bs_dir[PATH_MAX];

	struct option options[] = {
		{"blockstore", required_argument, NULL, 'b'},
		{"daemon",     no_argument,       NULL, 'd'},
		{"id",         required_argument, NULL, 'i'},
		{"numshdms",   required_argument, NULL, 'n'},
		{0, 0, 0, 0}
	};

	int c;
	while((c = getopt_long(argc, argv, "db:i:n:", options, NULL)) != -1) {
		switch(c) {
			case 'b':
				strncpy(bs_dir, optarg, PATH_MAX);
				path_provided = true;
				break;
			case 'd':
				opts->daemonise = true;
				break;
			case 'i':
				opts->shmd_id = atoi(optarg);
				break;
			case 'n':
				opts->nshmds = atoi(optarg);
				break;
			case '?':
				usage(argv[0]);
			default:
				exit(EXIT_FAILURE);
		}
	}

	if(!path_provided) {
		fprintf(stderr, "Please specify a blockstore path\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	} else {
		strcat(bs_dir, "/../"); // assume that the daemon is passed a path suffixed with /data/
		if(realpath(bs_dir, opts->bs_path) == NULL) {
			perror("realpath");
			exit(EXIT_FAILURE);
		}
	}
}
