// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include "imgtool.h"

struct command
{
	const char *name;
	int (*cmdproc)(const struct command *c, int argc, char *argv[]);
	const char *usage;
	int minargs;
	int maxargs;
	int lastargrepeats;
};

void reporterror(imgtoolerr_t err, const struct command *c, const char *format, const char *imagename,
	const char *filename, const char *newname, option_resolution *opts);

#ifdef MAME_DEBUG
int cmd_testsuite(struct command *c, int argc, char *argv[]);
#endif
