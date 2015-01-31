//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include "sdlinc.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

#define INCL_DOS
#include <os2.h>

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

//============================================================
//  osd_get_clipboard_text
//    - used in MESS
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	return result;
}

//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	struct stat st;

	err = stat(path, &st);

	if( err == -1) return NULL;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *) osd_malloc_array(sizeof(*result) + strlen(path) + 1);
	strcpy(((char *) result) + sizeof(*result), path);
	result->name = ((char *) result) + sizeof(*result);
	result->type = S_ISDIR(st.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	result->size = (UINT64)st.st_size;

	return result;
}

//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	static char szDrive[] = "A:\\";

	ULONG ulCurDisk;
	ULONG ulDriveMap;

	DosQueryCurrentDisk(&ulCurDisk, &ulDriveMap);

	szDrive[ 0 ] = 'A';
	while(idx--) {
		do
		{
			ulDriveMap >>= 1;
			szDrive[ 0 ]++;
		} while(ulDriveMap && (ulDriveMap & 1) == 0);
		if (!ulDriveMap) return NULL;
	}

	return szDrive;
}

//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	*dst = (char *)osd_malloc_array(CCHMAXPATH + 1);
	if (*dst == NULL)
		return FILERR_OUT_OF_MEMORY;

	_abspath(*dst, path, CCHMAXPATH + 1);
	return FILERR_NONE;
}
