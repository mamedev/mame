/**
 * resizable string buffer
 *
 * (c) 2017-2020 Steve Bennett <steveb@workware.net.au>
 *
 * See utf8.c for licence details.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#ifndef STRINGBUF_H
#include "stringbuf.h"
#endif
#ifdef USE_UTF8
#ifndef UTF8_UTIL_H
#include "utf8.h"
#endif
#endif

#define SB_INCREMENT 200

stringbuf *sb_alloc(void)
{
	stringbuf *sb = (stringbuf *)malloc(sizeof(*sb));
	sb->remaining = 0;
	sb->last = 0;
#ifdef USE_UTF8
	sb->chars = 0;
#endif
	sb->data = NULL;

	return(sb);
}

void sb_free(stringbuf *sb)
{
	if (sb) {
		free(sb->data);
	}
	free(sb);
}

static void sb_realloc(stringbuf *sb, int newlen)
{
	sb->data = (char *)realloc(sb->data, newlen);
	sb->remaining = newlen - sb->last;
}

void sb_append(stringbuf *sb, const char *str)
{
	sb_append_len(sb, str, strlen(str));
}

void sb_append_len(stringbuf *sb, const char *str, int len)
{
	if (sb->remaining < len + 1) {
		sb_realloc(sb, sb->last + len + 1 + SB_INCREMENT);
	}
	memcpy(sb->data + sb->last, str, len);
	sb->data[sb->last + len] = 0;

	sb->last += len;
	sb->remaining -= len;
#ifdef USE_UTF8
	sb->chars += utf8_strlen(str, len);
#endif
}

char *sb_to_string(stringbuf *sb)
{
	if (sb->data == NULL) {
		/* Return an allocated empty string, not null */
		return strdup("");
	}
	else {
		/* Just return the data and free the stringbuf structure */
		char *pt = sb->data;
		free(sb);
		return pt;
	}
}

/* Insert and delete operations */

/* Moves up all the data at position 'pos' and beyond by 'len' bytes
 * to make room for new data
 *
 * Note: Does *not* update sb->chars
 */
static void sb_insert_space(stringbuf *sb, int pos, int len)
{
	assert(pos <= sb->last);

	/* Make sure there is enough space */
	if (sb->remaining < len) {
		sb_realloc(sb, sb->last + len + SB_INCREMENT);
	}
	/* Now move it up */
	memmove(sb->data + pos + len, sb->data + pos, sb->last - pos);
	sb->last += len;
	sb->remaining -= len;
	/* And null terminate */
	sb->data[sb->last] = 0;
}

/**
 * Move down all the data from pos + len, effectively
 * deleting the data at position 'pos' of length 'len'
 */
static void sb_delete_space(stringbuf *sb, int pos, int len)
{
	assert(pos < sb->last);
	assert(pos + len <= sb->last);

#ifdef USE_UTF8
	sb->chars -= utf8_strlen(sb->data + pos, len);
#endif

	/* Now move it up */
	memmove(sb->data + pos, sb->data + pos + len, sb->last - pos - len);
	sb->last -= len;
	sb->remaining += len;
	/* And null terminate */
	sb->data[sb->last] = 0;
}

void sb_insert(stringbuf *sb, int index, const char *str)
{
	if (index >= sb->last) {
		/* Inserting after the end of the list appends. */
		sb_append(sb, str);
	}
	else {
		int len = strlen(str);

		sb_insert_space(sb, index, len);
		memcpy(sb->data + index, str, len);
#ifdef USE_UTF8
		sb->chars += utf8_strlen(str, len);
#endif
	}
}

/**
 * Delete the bytes at index 'index' for length 'len'
 * Has no effect if the index is past the end of the list.
 */
void sb_delete(stringbuf *sb, int index, int len)
{
	if (index < sb->last) {
		char *pos = sb->data + index;
		if (len < 0) {
			len = sb->last;
		}

		sb_delete_space(sb, pos - sb->data, len);
	}
}

void sb_clear(stringbuf *sb)
{
	if (sb->data) {
		/* Null terminate */
		sb->data[0] = 0;
		sb->last = 0;
#ifdef USE_UTF8
		sb->chars = 0;
#endif
	}
}
