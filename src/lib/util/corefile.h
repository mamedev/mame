// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corefile.h

    Core file I/O interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef __COREFILE_H__
#define __COREFILE_H__

#include <stdarg.h>
#include "corestr.h"
#include <string>
#include "coretmpl.h"



/***************************************************************************
    ADDITIONAL OPEN FLAGS
***************************************************************************/

#define OPEN_FLAG_NO_BOM        0x0100      /* don't output BOM */

#define FCOMPRESS_NONE          0           /* no compression */
#define FCOMPRESS_MIN           1           /* minimal compression */
#define FCOMPRESS_MEDIUM        6           /* standard compression */
#define FCOMPRESS_MAX           9           /* maximum compression */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct core_file;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- file open/close ----- */

/* open a file with the specified filename */
file_error core_fopen(const char *filename, UINT32 openflags, core_file **file);

/* open a RAM-based "file" using the given data and length (read-only) */
file_error core_fopen_ram(const void *data, size_t length, UINT32 openflags, core_file **file);

/* open a RAM-based "file" using the given data and length (read-only), copying the data */
file_error core_fopen_ram_copy(const void *data, size_t length, UINT32 openflags, core_file **file);

/* close an open file */
void core_fclose(core_file *file);

/* enable/disable streaming file compression via zlib; level is 0 to disable compression, or up to 9 for max compression */
file_error core_fcompress(core_file *file, int level);



/* ----- file positioning ----- */

/* adjust the file pointer within the file */
int core_fseek(core_file *file, INT64 offset, int whence);

/* return the current file pointer */
UINT64 core_ftell(core_file *file);

/* return true if we are at the EOF */
int core_feof(core_file *file);

/* return the total size of the file */
UINT64 core_fsize(core_file *file);



/* ----- file read ----- */

/* standard binary read from a file */
UINT32 core_fread(core_file *file, void *buffer, UINT32 length);

/* read one character from the file */
int core_fgetc(core_file *file);

/* put back one character from the file */
int core_ungetc(int c, core_file *file);

/* read a full line of text from the file */
char *core_fgets(char *s, int n, core_file *file);

/* get a pointer to a buffer that holds the full file data in RAM */
/* this function may cause the full file data to be read */
const void *core_fbuffer(core_file *file);

/* open a file with the specified filename, read it into memory, and return a pointer */
file_error core_fload(const char *filename, void **data, UINT32 *length);
file_error core_fload(const char *filename, dynamic_buffer &data);



/* ----- file write ----- */

/* standard binary write to a file */
UINT32 core_fwrite(core_file *file, const void *buffer, UINT32 length);

/* write a line of text to the file */
int core_fputs(core_file *f, const char *s);

/* printf-style text write to a file */
int core_vfprintf(core_file *f, const char *fmt, va_list va);
int CLIB_DECL core_fprintf(core_file *f, const char *fmt, ...) ATTR_PRINTF(2,3);

/* file truncation */
file_error core_truncate(core_file *f, UINT64 offset);



/* ----- filename utilities ----- */

/* extract the base part of a filename (remove extensions and paths) */
std::string core_filename_extract_base(const char *name, bool strip_extension = false);

/* true if the given filename ends with a particular extension */
int core_filename_ends_with(const char *filename, const char *extension);


#endif  /* __COREFILE_H__ */
