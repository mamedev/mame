/* linenoise.h -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * See linenoise.c for more information.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LINENOISE_H
#define __LINENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_COMPLETION
typedef struct linenoiseCompletions {
  size_t len;
  char **cvec;
} linenoiseCompletions;

/*
 * The callback type for tab completion handlers.
 */
typedef void(linenoiseCompletionCallback)(const char *prefix, linenoiseCompletions *comp, void *userdata);

/*
 * Sets the current tab completion handler and returns the previous one, or NULL
 * if no prior one has been set.
 */
linenoiseCompletionCallback * linenoiseSetCompletionCallback(linenoiseCompletionCallback *comp, void *userdata);

/*
 * Adds a copy of the given string to the given completion list. The copy is owned
 * by the linenoiseCompletions object.
 */
void linenoiseAddCompletion(linenoiseCompletions *comp, const char *str);

typedef char*(linenoiseHintsCallback)(const char *, int *color, int *bold, void *userdata);
typedef void(linenoiseFreeHintsCallback)(void *hint, void *userdata);
void linenoiseSetHintsCallback(linenoiseHintsCallback *callback, void *userdata);
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback *callback);

#endif

/*
 * Prompts for input using the given string as the input
 * prompt. Returns when the user has tapped ENTER or (on an empty
 * line) EOF (Ctrl-D on Unix, Ctrl-Z on Windows). Returns either
 * a copy of the entered string (for ENTER) or NULL (on EOF).  The
 * caller owns the returned string and must eventually free() it.
 */
char *linenoise(const char *prompt);

/**
 * Like linenoise() but starts with an initial buffer.
 */
char *linenoiseWithInitial(const char *prompt, const char *initial);

/**
 * Clear the screen.
 */
void linenoiseClearScreen(void);

/*
 * Adds a copy of the given line of the command history.
 */
int linenoiseHistoryAdd(const char *line);

/*
 * Sets the maximum length of the command history, in lines.
 * If the history is currently longer, it will be trimmed,
 * retaining only the most recent entries. If len is 0 or less
 * then this function does nothing.
 */
int linenoiseHistorySetMaxLen(int len);

/*
 * Returns the current maximum length of the history, in lines.
 */
int linenoiseHistoryGetMaxLen(void);

/*
 * Saves the current contents of the history to the given file.
 * Returns 0 on success.
 */
int linenoiseHistorySave(const char *filename);

/*
 * Replaces the current history with the contents
 * of the given file.  Returns 0 on success.
 */
int linenoiseHistoryLoad(const char *filename);

/*
 * Frees all history entries, clearing the history.
 */
void linenoiseHistoryFree(void);

/*
 * Returns a pointer to the list of history entries, writing its
 * length to *len if len is not NULL. The memory is owned by linenoise
 * and must not be freed.
 */
char **linenoiseHistory(int *len);

/*
 * Returns the number of display columns in the current terminal.
 */
int linenoiseColumns(void);

/**
 * Enable or disable multiline mode (disabled by default)
 */
void linenoiseSetMultiLine(int enableml);

#ifdef __cplusplus
}
#endif

#endif /* __LINENOISE_H */
