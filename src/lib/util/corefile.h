/***************************************************************************

    corefile.h

    Core file I/O interface functions and definitions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __COREFILE_H__
#define __COREFILE_H__

#include <stdarg.h>
#include "osdcore.h"
#include "astring.h"



/***************************************************************************
    ADDITIONAL OPEN FLAGS
***************************************************************************/

#define OPEN_FLAG_NO_BOM		0x0100		/* don't output BOM */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _core_file core_file;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- file open/close ----- */

/* open a file with the specified filename */
file_error core_fopen(const char *filename, UINT32 openflags, core_file **file);

/* open a RAM-based "file" using the given data and length (read-only) */
file_error core_fopen_ram(const void *data, size_t length, UINT32 openflags, core_file **file);

/* close an open file */
void core_fclose(core_file *file);



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



/* ----- file write ----- */

/* standard binary write to a file */
UINT32 core_fwrite(core_file *file, const void *buffer, UINT32 length);

/* write a line of text to the file */
int core_fputs(core_file *f, const char *s);

/* printf-style text write to a file */
int core_vfprintf(core_file *f, const char *fmt, va_list va);
int CLIB_DECL core_fprintf(core_file *f, const char *fmt, ...);



/* ----- filename utilities ----- */

/* extract the base part of a filename (remove extensions and paths) */
astring *core_filename_extract_base(astring *result, const char *name, int strip_extension);

/* true if the given filename ends with a particular extension */
int core_filename_ends_with(const char *filename, const char *extension);


#endif	/* __COREFILE_H__ */
