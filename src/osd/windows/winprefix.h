//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#define _WIN32_WINNT 0x0501

#ifdef MALLOC_DEBUG
#include <stdlib.h>
#include <malloc.h>

// override malloc/calloc/realloc/free to track file/line
void *malloc_file_line(size_t size, const char *file, int line);
void *calloc_file_line(size_t size, size_t count, const char *FILE, int line);
void *realloc_file_line(void *memory, size_t size, const char *file, int line);
void free_file_line(void *memory, const char *file, int line);

#undef malloc
#define malloc(x) malloc_file_line(x, __FILE__, __LINE__)
#undef calloc
#define calloc(x,y) calloc_file_line(x, y, __FILE__, __LINE__)
#undef realloc
#define realloc(x,y) realloc_file_line(x, y, __FILE__, __LINE__)
#undef free
#define free(x) free_file_line(x, __FILE__, __LINE__)
#endif

#ifdef _MSC_VER
#define alloca _alloca
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca	__builtin_alloca
#endif
#endif

#define PATH_SEPARATOR		"\\"
