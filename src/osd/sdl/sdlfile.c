//============================================================
//
//  fileio.c - SDL file access functions
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif
#ifndef SDLMAME_BSD
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// MAME headers
#include "osdcore.h"
#include "sdlos.h"

#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

static UINT32 create_path_recursive(char *path);

#define NO_ERROR	(0)

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct _osd_file
{
	int		handle;
	char		filename[1];
};


//============================================================
//  error_to_file_error
//  (does filling this out on non-Windows make any sense?)
//============================================================

static file_error error_to_file_error(UINT32 error)
{
	switch (error)
	{
	case ENOENT:
	case ENOTDIR:
		return FILERR_NOT_FOUND;

	case EACCES:
	case EROFS:
	#ifndef SDLMAME_WIN32
	case ETXTBSY:
	#endif
	case EEXIST:
	case EPERM:
	case EISDIR:
	case EINVAL:
		return FILERR_ACCESS_DENIED;

	case ENFILE:
	case EMFILE:
		return FILERR_TOO_MANY_FILES;

	default:
		return FILERR_FAILURE;
	}
}


//============================================================
//  osd_open
//============================================================

file_error osd_open(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	UINT32 access;
	const char *src;
	char *dst;
        #if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
	struct stat st;
	#else
	struct stat64 st;
	#endif
	char *tmpstr, *envstr;
	int i, j;
	file_error filerr = FILERR_NONE;

	tmpstr = NULL;

	// allocate a file object, plus space for the converted filename
	*file = (osd_file *) osd_malloc(sizeof(**file) + sizeof(char) * strlen(path));
	if (*file == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto error;
	}

	// convert the path into something compatible
	dst = (*file)->filename;
	for (src = path; *src != 0; src++)
		*dst++ = (*src == INVPATHSEPCH) ? PATHSEPCH : *src;
	*dst++ = 0;

	// select the file open modes
	if (openflags & OPEN_FLAG_WRITE)
	{
		access = (openflags & OPEN_FLAG_READ) ? O_RDWR : O_WRONLY;
		access |= (openflags & OPEN_FLAG_CREATE) ? (O_CREAT | O_TRUNC) : 0;
	}
	else if (openflags & OPEN_FLAG_READ)
	{
		access = O_RDONLY;
	}
	else
	{
		filerr = FILERR_INVALID_ACCESS;
		goto error;
	}

	tmpstr = (char *) osd_malloc(strlen((*file)->filename)+1);
	strcpy(tmpstr, (*file)->filename);

	// does path start with an environment variable?
	if (tmpstr[0] == '$')
	{
		char *envval;
		envstr = (char *) osd_malloc(strlen(tmpstr)+1);

		strcpy(envstr, tmpstr);

		i = 0;
		while (envstr[i] != PATHSEPCH && envstr[i] != 0 && envstr[i] != '.')
		{
			i++;
		}

		envstr[i] = '\0';

		envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
			j = strlen(envval) + strlen(tmpstr) + 1;
			osd_free(tmpstr);
			tmpstr = (char *) osd_malloc(j);

			// start with the value of $HOME
			strcpy(tmpstr, envval);
			// replace the null with a path separator again
			envstr[i] = PATHSEPCH;
			// append it
			strcat(tmpstr, &envstr[i]);
		}
		else
			fprintf(stderr, "Warning: osd_open environment variable %s not found.\n", envstr);
		osd_free(envstr);
	}

	#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
	access |= O_BINARY;
	#endif

	// attempt to open the file
        #if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
	(*file)->handle = open(tmpstr, access, 0666);
	#else
	(*file)->handle = open64(tmpstr, access, 0666);
	#endif
	if ((*file)->handle == -1)
	{
		// create the path if necessary
		if ((openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			char *pathsep = strrchr(tmpstr, PATHSEPCH);
			if (pathsep != NULL)
			{
				int error;

				// create the path up to the file
				*pathsep = 0;
				error = create_path_recursive(tmpstr);
				*pathsep = PATHSEPCH;

				// attempt to reopen the file
				if (error == NO_ERROR)
				{
		                        #if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
					(*file)->handle = open(tmpstr, access, 0666);
					#else
					(*file)->handle = open64(tmpstr, access, 0666);
					#endif
				}
			}
		}

		// if we still failed, clean up and osd_free
		if ((*file)->handle == -1)
		{
			osd_free(*file);
			*file = NULL;
			osd_free(tmpstr);
			return error_to_file_error(errno);
		}
	}

	// get the file size
        #if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
	fstat((*file)->handle, &st);
	#else
	fstat64((*file)->handle, &st);
	#endif

	*filesize = (UINT64)st.st_size;


error:
	// cleanup
	if (filerr != FILERR_NONE && *file != NULL)
	{
		osd_free(*file);
		*file = NULL;
	}
	if (tmpstr)
		osd_free(tmpstr);
	return filerr;
}


//============================================================
//  osd_read
//============================================================

file_error osd_read(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	ssize_t result;

#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD)
	result = pread(file->handle, buffer, count, offset);
	if (result < 0)
#elif defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_OS2)
	lseek(file->handle, (UINT32)offset&0xffffffff, SEEK_SET);
	result = read(file->handle, buffer, count);
	if (result < 0)
#elif defined(SDLMAME_UNIX)
	result = pread64(file->handle, buffer, count, offset);
	if (result < 0)
#else
#error Unknown SDL SUBARCH!
#endif
		return error_to_file_error(errno);
	if (actual != NULL)
		*actual = result;
	return FILERR_NONE;
}


//============================================================
//  osd_write
//============================================================

file_error osd_write(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	UINT32 result;

#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD)
	result = pwrite(file->handle, buffer, count, offset);
	if (!result)
#elif defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_OS2)
	lseek(file->handle, (UINT32)offset&0xffffffff, SEEK_SET);
	result = write(file->handle, buffer, count);
	if (!result)
#elif defined(SDLMAME_UNIX)
	result = pwrite64(file->handle, buffer, count, offset);
	if (!result)
#else
#error Unknown SDL SUBARCH!
#endif
		return error_to_file_error(errno);

	if (actual != NULL)
		*actual = result;
	return FILERR_NONE;
}


//============================================================
//  osd_close
//============================================================

file_error osd_close(osd_file *file)
{
	// close the file handle and free the file structure
	close(file->handle);
	osd_free(file);
	return FILERR_NONE;
}

//============================================================
//  osd_rmfile
//============================================================

file_error osd_rmfile(const char *filename)
{
	if (unlink(filename) == -1)
	{
		return error_to_file_error(errno);
	}

	return FILERR_NONE;
}

//============================================================
//  create_path_recursive
//============================================================

static UINT32 create_path_recursive(char *path)
{
	char *sep = strrchr(path, PATHSEPCH);
	UINT32 filerr;
	struct stat st;

	// if there's still a separator, and it's not the root, nuke it and recurse
	if (sep != NULL && sep > path && sep[0] != ':' && sep[-1] != PATHSEPCH)
	{
		*sep = 0;
		filerr = create_path_recursive(path);
		*sep = PATHSEPCH;
		if (filerr != NO_ERROR)
			return filerr;
	}

	// if the path already exists, we're done
	if (!stat(path, &st))
		return NO_ERROR;

	// create the path
	#ifdef SDLMAME_WIN32
	if (mkdir(path) != 0)
	#else
	if (mkdir(path, 0777) != 0)
	#endif
		return error_to_file_error(errno);
	return NO_ERROR;
}

//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	return FALSE;		// no, no way, huh-uh, forget it
}

/*============================================================ */
/*      osd_is_path_separator */
/*============================================================ */

int osd_is_path_separator(char c)
{
        return (c == '/') || (c == '\\');
}

/*============================================================ */
/*      osd_is_absolute_path */
/*============================================================ */
int osd_is_absolute_path(const char *path)
{
        int result;

        if (osd_is_path_separator(path[0]))
                result = TRUE;
#if !defined(SDLMAME_WIN32) && !defined(SDLMAME_OS2)
        else if (path[0] == '.')
                result = TRUE;
#else
	#ifndef UNDER_CE
	else if (*path && path[1] == ':')
		result = TRUE;
	#endif
#endif
        else
                result = FALSE;
        return result;
}

/* not used anywhere */
#if 0
//============================================================
//  osd_mkdir
//============================================================

file_error osd_mkdir(const char *dir)
{
	#ifdef SDLMAME_WIN32
	if (mkdir(dir) != 0)
	#else
	if (mkdir(dir, 0666) != 0)
	#endif
	{
		return error_to_file_error(errno);
	}

	return FILERR_NONE;
}

//============================================================
//  osd_rmdir
//============================================================

static file_error osd_rmdir(const char *dir)
{
	if (rmdir(dir) != 0)
	{
		return error_to_file_error(errno);
	}

	return FILERR_NONE;
}
#endif

// these are MESS specific - DO NOT TOUCH!!@!
#ifdef MESS

//============================================================
//  osd_copyfile
//  FIXME: this will not work with blanks in filename ...
//============================================================

file_error osd_copyfile(const char *destfile, const char *srcfile)
{
	char command[1024];

	sprintf(command, "cp %s %s\n", srcfile, destfile);
	system(command);

	return FILERR_NONE;
}

//============================================================
//  osd_stat
//============================================================

#ifndef SDLMAME_WIN32
osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
	struct stat st;
	#else
	struct stat64 st;
	#endif

	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
	err = stat(path, &st);
	#else
	err = stat64(path, &st);
	#endif

	if( err == -1) return NULL;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *) osd_malloc(sizeof(*result) + strlen(path) + 1);
	strcpy(((char *) result) + sizeof(*result), path);
	result->name = ((char *) result) + sizeof(*result);
	result->type = S_ISDIR(st.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	result->size = (UINT64)st.st_size;

	return result;
}
#endif
#endif

//============================================================
//   osd_getcurdir
//============================================================

file_error osd_getcurdir(char *buffer, size_t buffer_len)
{
	file_error filerr = FILERR_NONE;

	if (getcwd(buffer, buffer_len) == NULL)
	{
		filerr = FILERR_FAILURE;
	}

	if( filerr != FILERR_FAILURE )
	{
		if( strcmp( &buffer[strlen(buffer)-1], PATH_SEPARATOR ) != 0 )
		{
		        strncat( buffer, PATH_SEPARATOR, buffer_len );
		}
	}

	return filerr;
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
		*dst = (char *)malloc(strlen(path_buffer)+strlen(path)+3);

		// if it's already a full path, just pass it through
		if (path[0] == '/')
		{
			strcpy(*dst, path);
		}
		else
		{
			sprintf(*dst, "%s%s%s", path_buffer, PATH_SEPARATOR, path);
		}
	}

	return err;
}

//============================================================
//  osd_setcurdir
//============================================================

file_error osd_setcurdir(const char *dir)
{
	file_error filerr = FILERR_NONE;

	if (chdir(dir) != 0)
	{
		filerr = FILERR_FAILURE;
	}

	return filerr;
}

//============================================================
//  osd_basename
//============================================================

char *osd_basename(char *filename)
{
	char *c;

	// NULL begets NULL
	if (!filename)
		return NULL;

	// start at the end and return when we hit a slash or colon
	for (c = filename + strlen(filename) - 1; c >= filename; c--)
		if (*c == '\\' || *c == '/' || *c == ':')
			return c + 1;

	// otherwise, return the whole thing
	return filename;
}

//============================================================
//  osd_dirname
//============================================================

char *osd_dirname(const char *filename)
{
	char *dirname;
	char *c;

	// NULL begets NULL
	if (!filename)
		return NULL;

	// allocate space for it
	dirname = (char*)malloc(strlen(filename) + 1);
	if (!dirname)
		return NULL;

	// copy in the name
	strcpy(dirname, filename);

	// search backward for a slash or a colon
	for (c = dirname + strlen(dirname) - 1; c >= dirname; c--)
		if (*c == '\\' || *c == '/' || *c == ':')
		{
			// found it: NULL terminate and return
			*(c + 1) = 0;
			return dirname;
		}

	// otherwise, return an empty string
	dirname[0] = 0;
	return dirname;
}

// no idea what these are for, just cheese them
int osd_num_devices(void)
{
	return 1;
}

const char *osd_get_device_name(int idx)
{
	return "/";
}

//============================================================
//  osd_get_temp_filename
//============================================================

file_error osd_get_temp_filename(char *buffer, size_t buffer_len, const char *basename)
{
	char tempbuf[512];

	if (!basename)
		basename = "tempfile";

	sprintf(tempbuf, "/tmp/%s", basename);
	unlink(tempbuf);

	strncpy(buffer, tempbuf, buffer_len);

	return FILERR_NONE;
}

//============================================================
//  osd_mkdir
//============================================================

file_error osd_mkdir(const char *dir)
{
	file_error filerr = FILERR_NONE;

	#ifdef SDLMAME_WIN32
	if (mkdir(dir) != 0)
	#else
	if (mkdir(dir, 0700) != 0)
	#endif
	{
		filerr = FILERR_FAILURE;
	}

	return filerr;
}

//============================================================
//  osd_rmdir
//============================================================

file_error osd_rmdir(const char *dir)
{
	file_error filerr = FILERR_NONE;

	if (rmdir(dir) != 0)
	{
		filerr = FILERR_FAILURE;
	}

	return filerr;
}
char sdl_cwd[512];
void osd_get_emulator_directory(char *dir, size_t dir_size)
{
	strncpy(dir, sdl_cwd, dir_size);
}
