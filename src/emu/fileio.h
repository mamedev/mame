/***************************************************************************

    fileio.h

    Core file I/O interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include "corefile.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* search paths */
#define SEARCHPATH_RAW             NULL
#define SEARCHPATH_LANGUAGE        NULL
#define SEARCHPATH_DEBUGLOG        NULL

#define SEARCHPATH_ROM             OPTION_ROMPATH
#define SEARCHPATH_IMAGE           OPTION_ROMPATH
#define SEARCHPATH_HASH            OPTION_HASHPATH
#define SEARCHPATH_SAMPLE          OPTION_SAMPLEPATH
#define SEARCHPATH_ARTWORK         OPTION_ARTPATH
#define SEARCHPATH_CTRLR           OPTION_CTRLRPATH
#define SEARCHPATH_INI             OPTION_INIPATH
#define SEARCHPATH_FONT            OPTION_FONTPATH
#define SEARCHPATH_CHEAT           OPTION_CHEATPATH
#define SEARCHPATH_CROSSHAIRPATH   OPTION_CROSSHAIRPATH

#define SEARCHPATH_IMAGE_DIFF      OPTION_DIFF_DIRECTORY
#define SEARCHPATH_NVRAM           OPTION_NVRAM_DIRECTORY
#define SEARCHPATH_CONFIG          OPTION_CFG_DIRECTORY
#define SEARCHPATH_INPUTLOG        OPTION_INPUT_DIRECTORY
#define SEARCHPATH_STATE           OPTION_STATE_DIRECTORY
#define SEARCHPATH_MEMCARD         OPTION_MEMCARD_DIRECTORY
#define SEARCHPATH_SCREENSHOT      OPTION_SNAPSHOT_DIRECTORY
#define SEARCHPATH_MOVIE           OPTION_SNAPSHOT_DIRECTORY
#define SEARCHPATH_COMMENT         OPTION_COMMENT_DIRECTORY



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _mame_file mame_file;
typedef struct _mame_path mame_path;

typedef struct _core_options core_options;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core initialization ----- */

/* initialize the fileio system */
void fileio_init(running_machine *machine);



/* ----- file open/close ----- */

/* open a file in the given search path with the specified filename */
file_error mame_fopen(const char *searchpath, const char *filename, UINT32 openflags, mame_file **file);

/* open a file in the given search path with the specified filename or a matching CRC */
file_error mame_fopen_crc(const char *searchpath, const char *filename, UINT32 crc, UINT32 openflags, mame_file **file);

/* open a file in the given search path with the specified filename, using the specified options */
file_error mame_fopen_options(core_options *opts, const char *searchpath, const char *filename, UINT32 openflags, mame_file **file);

/* open a file in the given search path with the specified filename or a matching CRC, using the specified options */
file_error mame_fopen_crc_options(core_options *opts, const char *searchpath, const char *filename, UINT32 crc, UINT32 openflags, mame_file **file);

/* open a "file" which is actually data in RAM */
file_error mame_fopen_ram(const void *data, UINT32 length, UINT32 openflags, mame_file **file);

/* close an open file */
void mame_fclose(mame_file *file);

/* close an open file, and open the next entry in the original searchpath*/
file_error mame_fclose_and_open_next(mame_file **file, const char *filename, UINT32 openflags);

/* enable/disable streaming file compression via zlib; level is 0 to disable compression, or up to 9 for max compression */
file_error mame_fcompress(mame_file *file, int compress);



/* ----- file positioning ----- */

/* adjust the file pointer within the file */
int mame_fseek(mame_file *file, INT64 offset, int whence);

/* return the current file pointer */
UINT64 mame_ftell(mame_file *file);

/* return true if we are at the EOF */
int mame_feof(mame_file *file);

/* return the total size of the file */
UINT64 mame_fsize(mame_file *file);



/* ----- file read ----- */

/* standard binary read from a file */
UINT32 mame_fread(mame_file *file, void *buffer, UINT32 length);

/* read one character from the file */
int mame_fgetc(mame_file *file);

/* put back one character from the file */
int mame_ungetc(int c, mame_file *file);

/* read a full line of text from the file */
char *mame_fgets(char *s, int n, mame_file *file);



/* ----- file write ----- */

/* standard binary write to a file */
UINT32 mame_fwrite(mame_file *file, const void *buffer, UINT32 length);

/* write a line of text to the file */
int mame_fputs(mame_file *f, const char *s);

/* printf-style text write to a file */
int CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...) ATTR_PRINTF(2,3);



/* ----- file enumeration ----- */

/* open a search path (multiple directories) for iteration */
mame_path *mame_openpath(core_options *opts, const char *searchpath);

/* return information about the next file in the search path */
const osd_directory_entry *mame_readpath(mame_path *path);

/* close an open seach path */
void mame_closepath(mame_path *path);



/* ----- file misc ----- */

/* return the core_file underneath the mame_file */
core_file *mame_core_file(mame_file *file);

/* return the full filename for a given mame_file */
const astring &mame_file_full_name(mame_file *file);

/* return a hash string for the file with the given functions */
const char *mame_fhash(mame_file *file, UINT32 functions);



#endif	/* __FILEIO_H__ */
