#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"

#ifndef NO_COMPLETION
void completion(const char *buf, linenoiseCompletions *lc, void *userdata) {
    (void)userdata;
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    }
}

char *hints(const char *buf, int *color, int *bold, void *userdata) {
    (void)userdata;
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World";
    }
    return NULL;
}
#endif

int main(int argc, char *argv[]) {
    const char *prompt = "hello> ";
    char *line;
    char *prgname = argv[0];
	const char *initial;

    /* Parse options, with --multiline we enable multi line editing. */
    while(argc > 1 && argv[1][0] == '-') {
        argc--;
        argv++;
        if (!strcmp(*argv,"--multiline")) {
            linenoiseSetMultiLine(1);
            printf("Multi-line mode enabled.\n");
        } else if (!strcmp(*argv,"--fancyprompt")) {
            prompt = "\x1b[1;31m\xf0\xa0\x8a\x9d-\xc2\xb5hello>\x1b[0m ";
        } else if (!strcmp(*argv,"--prompt") && argc > 1) {
            argc--;
            argv++;
            prompt = *argv;
        } else {
            fprintf(stderr, "Usage: %s [--multiline] [--fancyprompt] [--prompt text]\n", prgname);
            exit(1);
        }
    }

#ifndef NO_COMPLETION
    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion, NULL);
    linenoiseSetHintsCallback(hints, NULL);
#endif

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

	initial = (argc > 1) ? argv[1] : "";

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    while((line = linenoiseWithInitial(prompt, initial)) != NULL) {
		initial = "";
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
            printf("echo: '%s'\n", line);
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            linenoiseHistorySetMaxLen(len);
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
