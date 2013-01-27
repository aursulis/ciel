#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

struct shmd_options {
	bool daemonise;
	const char *bs_path;
};

void parse_options(int argc, char **argv, struct shmd_options *opts);

#endif
