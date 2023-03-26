#ifndef STRINGBUF_H
#define STRINGBUF_H
/**
 * resizable string buffer
 *
 * (c) 2017-2020 Steve Bennett <steveb@workware.net.au>
 *
 * See utf8.c for licence details.
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @file
 * A stringbuf is a resizing, null terminated string buffer.
 *
 * The buffer is reallocated as necessary.
 *
 * In general it is *not* OK to call these functions with a NULL pointer
 * unless stated otherwise.
 *
 * If USE_UTF8 is defined, supports utf8.
 */

/**
 * The stringbuf structure should not be accessed directly.
 * Use the functions below.
 */
typedef struct {
	int remaining;	/**< Allocated, but unused space */
	int last;		/**< Index of the null terminator (and thus the length of the string) */
#ifdef USE_UTF8
	int chars;		/**< Count of characters */
#endif
	char *data;		/**< Allocated memory containing the string or NULL for empty */
} stringbuf;

/**
 * Allocates and returns a new stringbuf with no elements.
 */
stringbuf *sb_alloc(void);

/**
 * Frees a stringbuf.
 * It is OK to call this with NULL.
 */
void sb_free(stringbuf *sb);

/**
 * Returns an allocated copy of the stringbuf
 */
stringbuf *sb_copy(stringbuf *sb);

/**
 * Returns the byte length of the buffer.
 * 
 * Returns 0 for both a NULL buffer and an empty buffer.
 */
static inline int sb_len(stringbuf *sb) {
	return sb->last;
}

/**
 * Returns the utf8 character length of the buffer.
 * 
 * Returns 0 for both a NULL buffer and an empty buffer.
 */
static inline int sb_chars(stringbuf *sb) {
#ifdef USE_UTF8
	return sb->chars;
#else
	return sb->last;
#endif
}

/**
 * Appends a null terminated string to the stringbuf
 */
void sb_append(stringbuf *sb, const char *str);

/**
 * Like sb_append() except does not require a null terminated string.
 * The length of 'str' is given as 'len'
 *
 * Note that in utf8 mode, characters will *not* be counted correctly
 * if a partial utf8 sequence is added with sb_append_len()
 */
void sb_append_len(stringbuf *sb, const char *str, int len);

/**
 * Returns a pointer to the null terminated string in the buffer.
 *
 * Note this pointer only remains valid until the next modification to the
 * string buffer.
 *
 * The returned pointer can be used to update the buffer in-place
 * as long as care is taken to not overwrite the end of the buffer.
 */
static inline char *sb_str(const stringbuf *sb)
{
	return sb->data;
}

/**
 * Inserts the given string *before* (zero-based) byte 'index' in the stringbuf.
 * If index is past the end of the buffer, the string is appended,
 * just like sb_append()
 */
void sb_insert(stringbuf *sb, int index, const char *str);

/**
 * Delete 'len' bytes in the string at the given index.
 *
 * Any bytes past the end of the buffer are ignored.
 * The buffer remains null terminated.
 *
 * If len is -1, deletes to the end of the buffer.
 */
void sb_delete(stringbuf *sb, int index, int len);

/**
 * Clear to an empty buffer.
 */
void sb_clear(stringbuf *sb);

/**
 * Return an allocated copy of buffer and frees 'sb'.
 *
 * If 'sb' is empty, returns an allocated copy of "".
 */
char *sb_to_string(stringbuf *sb);

#ifdef __cplusplus
}
#endif

#endif
