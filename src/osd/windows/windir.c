/***************************************************************************

    windir.c - Win32 OSD core directory access functions

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

// standard C headers
#include <stdio.h>
#include <ctype.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct _osd_directory
{
	HANDLE				find;					// handle to the finder
	int					is_first;				// TRUE if this is the first entry
	osd_directory_entry	entry;					// current entry's data
	WIN32_FIND_DATA		data;					// current raw data
};



//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	osd_directory *dir = NULL;
	TCHAR *t_dirname = NULL;
	TCHAR *dirfilter = NULL;
	size_t dirfilter_size;

	// allocate memory to hold the osd_tool_directory structure
	dir = malloc(sizeof(*dir));
	if (dir == NULL)
		goto error;
	memset(dir, 0, sizeof(*dir));

	// initialize the structure
	dir->find = INVALID_HANDLE_VALUE;
	dir->is_first = TRUE;

	// convert the path to TCHARs
	t_dirname = tstring_from_utf8(dirname);
	if (t_dirname == NULL)
		goto error;

	// append \*.* to the directory name
	dirfilter_size = _tcslen(t_dirname) + 5;
	dirfilter = (TCHAR *)malloc(dirfilter_size * sizeof(*dirfilter));
	if (dirfilter == NULL)
		goto error;
	_sntprintf(dirfilter, dirfilter_size, TEXT("%s\\*.*"), t_dirname);

	// attempt to find the first file
	dir->find = FindFirstFile(dirfilter, &dir->data);

error:
	// cleanup
	if (t_dirname != NULL)
		free(t_dirname);
	if (dirfilter != NULL)
		free(dirfilter);
	if (dir != NULL && dir->find == INVALID_HANDLE_VALUE)
	{
		free(dir);
		dir = NULL;
	}
	return dir;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	// if we've previously allocated a name, free it now
	if (dir->entry.name != NULL)
	{
		free((void *)dir->entry.name);
		dir->entry.name = NULL;
	}

	// if this isn't the first file, do a find next
	if (!dir->is_first)
	{
		if (!FindNextFile(dir->find, &dir->data))
			return NULL;
	}

	// otherwise, just use the data we already had
	else
		dir->is_first = FALSE;

	// extract the data
	dir->entry.name = utf8_from_tstring(dir->data.cFileName);
	dir->entry.type = win_attributes_to_entry_type(dir->data.dwFileAttributes);
	dir->entry.size = dir->data.nFileSizeLow | ((UINT64) dir->data.nFileSizeHigh << 32);
	return (dir->entry.name != NULL) ? &dir->entry : NULL;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	// free any data associated
	if (dir->entry.name != NULL)
		free((void *)dir->entry.name);
	if (dir->find != INVALID_HANDLE_VALUE)
		FindClose(dir->find);
	free(dir);
}


//============================================================
//  osd_is_absolute_path
//============================================================

int osd_is_absolute_path(const char *path)
{
	int result = FALSE;
	TCHAR *t_path = tstring_from_utf8(path);
	if (t_path != NULL)
	{
		result = !PathIsRelative(t_path);
		free(t_path);
	}
	return result;
}

