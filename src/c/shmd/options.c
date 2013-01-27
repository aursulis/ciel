#include "options.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>

static void usage(const char *argv0)
{
	fprintf(stderr, "usage: %s OPTIONS\n", argv0);
	fprintf(stderr, "  -b PATH, --blockstore PATH : specify blockstore path\n");
	fprintf(stderr, "  -d,      --daemon          : run as daemon, log file in blockstore path\n");
}

void parse_options(int argc, char **argv, struct shmd_options *opts)
{
	opts->daemonise = false;
	opts->bs_path = NULL;

	struct option options[] = {
		{"blockstore", required_argument, NULL, 'b'},
		{"daemon",     no_argument,       NULL, 'd'},
		{0, 0, 0, 0}
	};

	int c;
	while((c = getopt_long(argc, argv, "db:", options, NULL)) != -1) {
		switch(c) {
			case 'b':
				opts->bs_path = optarg;
				break;
			case 'd':
				opts->daemonise = true;
				break;
			case '?':
				usage(argv[0]);
			default:
				exit(EXIT_FAILURE);
		}
	}

	if(opts->bs_path == NULL) {
		fprintf(stderr, "Please specify a blockstore path\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
}
