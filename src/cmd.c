#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "cmd.h"
#include "util.h"

static struct command {
	char *name;
	char *opts;
	char *summary;
	int (*func)(int argc, char **argv);
} cmds[100], *next_cmd = cmds;

#define cmd_for_each(com) for (com = cmds; com->func; com++)

void cmd_register(char *name, char *opts, char *summary,
		  int (*func)(int argc, char **argv))
{
	struct command *com = next_cmd++;

	assert((com - cmds) < array_size(cmds));

	com->name = name;
	com->opts = opts;
	com->summary = summary;
	com->func = func;
}

static struct command *find_command(char *name)
{
	struct command *com;

	cmd_for_each(com) {
		if (!strcmp(name, com->name))
			return com;
	}

	return NULL;
}

static void usage(void)
{
	struct command *com;
	int largest = 0;

	fprintf(stderr, "usage: scoutfs <command> [<args>]\n"
	       "Commands:\n");

	cmd_for_each(com)
		largest = max(strlen(com->name), largest);

	cmd_for_each(com) {
		fprintf(stderr, "  %*s %s\n  %*s %s\n",
			largest, com->name, com->opts,
			largest, "", com->summary);
	}
}

int cmd_execute(int argc, char **argv)
{
	struct command *com = NULL;
	int ret;

	if (argc > 1) {
		com = find_command(argv[1]);
		if (!com)
			fprintf(stderr, "scoutfs: unrecognized command: '%s'\n",
				argv[1]);
	}
	if (!com) {
		usage();
		return 1;
	}

	ret = com->func(argc - 1, argv + 1);
	if (ret < 0) {
		fprintf(stderr, "scoutfs: %s failed: %s (%d)\n",
			com->name, strerror(-ret), -ret);
		return 1;
	}

	return 0;
}
