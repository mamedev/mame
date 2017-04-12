/**
 * \file   path_helpers.c
 * \brief  Helper functions for path_getrelative.c and path_getabsolute.c
 */

#include "premake.h"
#include <string.h>

#define USE_DRIVE_LETTERS PLATFORM_WINDOWS

static int has_drive_letter(const char * path)
{
#if USE_DRIVE_LETTERS
	char ch = path[0];
	if (((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) && path[1] == ':')
		return 1;
#else
	(void)path;
#endif // USE_DRIVE_LETTERS

	return 0;
}

int is_absolute_path(const char * path)
{
	char ch = path[0];
	if (ch == '\0')
	{
		return 0;
	}

	if (ch == '/' || ch == '\\' || ch == '$')
	{
		return 1;
	}

	if (has_drive_letter(path))
	{
		return 1;
	}

	return 0;
}

static char *copy_current_directory(char * buffer, int bufsize)
{
#if PLATFORM_WINDOWS
	int len = GetCurrentDirectory(bufsize, buffer);
	int i;
	for (i = 0; i < len; i++)
		buffer[i] = (buffer[i] == '\\' ? '/' : buffer[i]);
	return buffer + len;
#else
	char *ptr = getcwd(buffer, bufsize);
	int len = ptr ? strlen(ptr) : 0;
	return buffer + len;
#endif
}

#if USE_DRIVE_LETTERS
static char *copy_current_drive_letter(char * buffer, int bufsize)
{
	int len = GetCurrentDirectory(bufsize, buffer);
	if (len >= 2 && buffer[1] == ':') {
		buffer[2] = '\0';
		return buffer + 2;
	}
	else {
		buffer[0] = '\0';
		return buffer;
	}
}
#endif // USE_DRIVE_LETTERS

static char *backtrack(char *path, char *start)
{
	for (; path > start; --path) {
		if (*path == '/') {
			*path = '\0';
			return path;
		}
#if USE_DRIVE_LETTERS
		else if (*path == ':' && (path == start + 1)) {
			path[1] = '/';
			path[2] = '\0';
			return path + 2;
		}
#endif
	}

	start[0] = '/';
	start[1] = '\0';
	return start + 1;
}

static char *concat(char *path, const char *pathTerm, const char *token, const char *tokenTerm)
{
	while (path < pathTerm && token < tokenTerm)
		*path++ = *token++;
	return path;
}

static char *copy(const char str[], char *buffer, int bufsize)
{
	int i = 0;
	for (; str[i] && i < bufsize - 1; i++)
		buffer[i] = str[i];
	buffer[i] = '\0';
	return buffer + i;
}

char *get_absolute_path(const char path[], char *buffer, int bufsize)
{
	// There must be room for at least a drive letter, colon and slash.
	if (bufsize < 4)
		return 0;

	char *dst  = buffer;
	char *term = buffer + bufsize - 1;

	// If the path started as relative, prepend the current working directory
	if (!is_absolute_path(path))
	{
		dst = copy_current_directory(buffer, bufsize);
	}
#if USE_DRIVE_LETTERS
	// Add drive letter to absolute paths without one
	else if (!has_drive_letter(path) && path[0] != '$')
	{
		dst = copy_current_drive_letter(buffer, bufsize);
	}
#endif

	// Process the path one segment at a time.
	int segment = 0;
	const char *src   = path;
	const char *token = src;
	for (; dst < term; ++src) {
		if (*src == '/' || *src == '\\' || *src == '\0') {
			int tokenlen = (int)(src - token);
			if (tokenlen == 0) {
				if (segment == 0)
					*dst++ = '/';
			}
			else if (tokenlen == 1 && token[0] == '.') {
				// do nothing
			}
			else if (tokenlen == 2 && token[0] == '.' && token[1] == '.') {
				dst = backtrack(dst - 1, buffer);
			}
			else {
				if (dst > buffer && dst[-1] != '/')
					*dst++ = '/';
				dst = concat(dst, term, token, src);
			}
			token = src + 1;
			++segment;
			if (*src == '\0')
				break;
		}
	}

#if USE_DRIVE_LETTERS
	// Make sure drive letter is lowercase
	buffer[0] = (char)tolower(buffer[0]);
#endif

	// Chop trailing slash
	if (dst > buffer && dst[-1] == '/')
		--dst;

	// Add a null terminator.
	if (dst < term)
		*dst = '\0';
	else
		*--dst = '\0';

	return dst;
}

char *get_relative_path(const char src[], const char dst[], char *buffer, int bufsize)
{
	// There must be room for at least a drive letter, colon and slash.
	if (bufsize < 4)
		return 0;

	// If the dst path starts with $, return it as an absolute path.
	if (dst[0] == '$')
		return copy(dst, buffer, bufsize);

	// Get the absolute source and destination paths.
	char srcBuf[PATH_BUFSIZE];
	char dstBuf[PATH_BUFSIZE];
	char *srcEnd = get_absolute_path(src, srcBuf, PATH_BUFSIZE - 1);
	char *dstEnd = get_absolute_path(dst, dstBuf, PATH_BUFSIZE - 1);

#if USE_DRIVE_LETTERS
	// If the paths have different drive letters, return the absolute path.
	if (srcBuf[0] != dstBuf[0])
		return copy(dstBuf, buffer, bufsize);
#endif

	// Append '/' to the end of each path.
	*srcEnd++ = '/'; *srcEnd = '\0';
	*dstEnd++ = '/'; *dstEnd = '\0';

	// Find the shared path prefix of the src and dst paths.
	int lastSlash = -1;
	int i;
	for (i = 0; srcBuf[i] == dstBuf[i]; i++) {
		if (srcBuf[i] == '/') {
			lastSlash = i;
		}
		else if (srcBuf[i] == '\0') {
			// Paths are identical, so return '.'
			buffer[0] = '.';
			buffer[1] = '\0';
			return buffer + 1;
		}
	}

	// For each remaining path segment in src after the last shared slash,
	// append '../' to the result.
	char *resultPtr  = buffer;
	char *resultTerm = buffer + bufsize;
	const char *ptr;
	for (ptr = srcBuf + lastSlash + 1; *ptr; ++ptr) {
		if (*ptr == '/' && resultPtr + 3 < resultTerm) {
			resultPtr[0] = '.';
			resultPtr[1] = '.';
			resultPtr[2] = '/';
			resultPtr += 3;
		}
	}

	// Append the portion of the destination path beyond the last shared
	// slash.
	char *dstPtr;
	for (dstPtr = dstBuf + lastSlash + 1; *dstPtr && resultPtr < resultTerm; )
		*resultPtr++ = *dstPtr++;

	// Remove the trailing slash.
	if (resultPtr[-1] == '/')
		--resultPtr;

	// Add a null terminator.
	if (resultPtr < resultTerm)
		*resultPtr = '\0';
	else
		*--resultPtr = '\0';

	return resultPtr;
}
