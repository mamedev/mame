/* Common functions and includes for our test programs. */

/*
 * Set feature macro to enable getline() and wcwidth().
 *
 * Please refer to section 2.2.1 of POSIX.1-2008:
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_02_01_02
 */
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "../utf8proc.h"

extern size_t lineno;

void check(int cond, const char *format, ...);
size_t skipspaces(const char *buf, size_t i);
size_t encode(char *dest, const char *buf);
