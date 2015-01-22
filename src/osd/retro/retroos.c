#include <stdlib.h>
#include <unistd.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <sys/stat.h>

// MAME headers
#include "osdcore.h"


//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	return result;
}

#if 1
//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	#if defined(SDLMAME_NO64BITIO) || defined(RETRO_AND) || defined(WIN32) || defined(SDLMAME_BSD)
	struct stat st;
	#else
	struct stat64 st;
	#endif

	#if defined(SDLMAME_NO64BITIO) || defined(RETRO_AND) || defined(WIN32) || defined(SDLMAME_BSD)
	err = stat(path, &st);
	#else
	err = stat64(path, &st);
	#endif

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
	if (idx!=0) return NULL;
	return "/";
}

//============================================================
//  osd_get_slider_list
//============================================================

const void *osd_get_slider_list()
{
	return NULL;
}

//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	file_error err;
	char path_buffer[512];

	err = FILERR_NONE;

	if (getcwd(path_buffer, 511) == NULL)
	{
		printf("osd_get_full_path: failed!\n");
		err = FILERR_FAILURE;
	}
	else
	{
		*dst = (char *)osd_malloc_array(strlen(path_buffer)+strlen(path)+3);

		// if it's already a full path, just pass it through
		if (path[0] == '/')
			strcpy(*dst, path);
		else
			sprintf(*dst, "%s%s%s", path_buffer, PATH_SEPARATOR, path);
	}

	return err;
}

#endif
