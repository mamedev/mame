// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlfile.c - SDL file access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#include "../windows/winfile.cpp"
#include "../windows/winutil.cpp"
#else

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif

#ifdef SDLMAME_WIN32
#define _FILE_OFFSET_BITS 64
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
#include <errno.h>

// MAME headers
#include "sdlfile.h"
#include "modules/lib/osdlib.h"

//============================================================
//  GLOBAL IDENTIFIERS
//============================================================

extern const char *sdlfile_socket_identifier;
extern const char *sdlfile_ptty_identifier;

//============================================================
//  CONSTANTS
//============================================================

#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

#define NO_ERROR    (0)

//============================================================
//  Prototypes
//============================================================

static UINT32 create_path_recursive(char *path);

//============================================================
//  error_to_file_error
//  (does filling this out on non-Windows make any sense?)
//============================================================

file_error error_to_file_error(UINT32 error)
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
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
	struct stat st;
	#else
	struct stat64 st;
	#endif
	char *tmpstr;
	file_error filerr = FILERR_NONE;

	tmpstr = NULL;

	// allocate a file object, plus space for the converted filename
	*file = (osd_file *) osd_malloc_array(sizeof(**file) + sizeof(char) * strlen(path));
	if (*file == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto error;
	}

	if (sdl_check_socket_path(path))
	{
		(*file)->type = SDLFILE_SOCKET;
		filerr = sdl_open_socket(path, openflags, file, filesize);
		goto error;
	}

	if (strlen(sdlfile_ptty_identifier) > 0 && strncmp(path, sdlfile_ptty_identifier, strlen(sdlfile_ptty_identifier)) == 0)
	{
		(*file)->type = SDLFILE_PTTY;
		filerr = sdl_open_ptty(path, openflags, file, filesize);
		goto error;
	}

	(*file)->type = SDLFILE_FILE;

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

	osd_subst_env(&tmpstr, (*file)->filename);

	#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
	access |= O_BINARY;
	#endif

	// attempt to open the file
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
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
					#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
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
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
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

	switch (file->type)
	{
		case SDLFILE_FILE:
#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD) || defined(SDLMAME_EMSCRIPTEN)
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

		case SDLFILE_SOCKET:
			return sdl_read_socket(file, buffer, offset, count, actual);

		case SDLFILE_PTTY:
			return sdl_read_ptty(file, buffer, offset, count, actual);

		default:
			return FILERR_FAILURE;
	}
}


//============================================================
//  osd_write
//============================================================

file_error osd_write(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	UINT32 result;

	switch (file->type)
	{
		case SDLFILE_FILE:
#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD) || defined(SDLMAME_EMSCRIPTEN)
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

		case SDLFILE_SOCKET:
			return sdl_write_socket(file, buffer, offset, count, actual);

		case SDLFILE_PTTY:
			return sdl_write_ptty(file, buffer, offset, count, actual);

		default:
			return FILERR_FAILURE;
	}
}

//============================================================
//  osd_openpty
//============================================================

file_error osd_openpty(osd_file **file, char *name, size_t name_len)
{
		file_error res;
		UINT64 filesize;

		if ((res = osd_open(sdlfile_ptty_identifier , 0 , file , &filesize)) != FILERR_NONE) {
				return res;
		}

		if ((res = sdl_slave_name_ptty(*file , name , name_len)) != FILERR_NONE) {
				osd_close(*file);
		}

		return res;
}

//============================================================
//  osd_truncate
//============================================================

file_error osd_truncate(osd_file *file, UINT64 offset)
{
	UINT32 result;

	if (!file || !file->handle)
		return FILERR_FAILURE;

	switch (file->type)
	{
		case SDLFILE_FILE:
			result = ftruncate(file->handle, offset);
			if (result)
				return error_to_file_error(errno);
			return FILERR_NONE;

		default:
			return FILERR_FAILURE;
	}
}


//============================================================
//  osd_close
//============================================================

file_error osd_close(osd_file *file)
{
	// close the file handle and free the file structure
	switch (file->type)
	{
		case SDLFILE_FILE:
			close(file->handle);
			osd_free(file);
			return FILERR_NONE;

		case SDLFILE_SOCKET:
			return sdl_close_socket(file);

		case SDLFILE_PTTY:
			return sdl_close_ptty(file);

		default:
			return FILERR_FAILURE;
	}
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
	return FALSE;       // no, no way, huh-uh, forget it
}

//============================================================
//  osd_is_path_separator
//============================================================

static int osd_is_path_separator(char c)
{
	return (c == PATHSEPCH) || (c == INVPATHSEPCH);
}

//============================================================
//  osd_is_absolute_path
//============================================================

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

//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	#if defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_DARWIN) || defined(SDLMAME_OS2)
	struct stat st;
	#else
	struct stat64 st;
	#endif

	#if defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_DARWIN) || defined(SDLMAME_OS2)
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
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	if (idx!=0) return NULL;
	return "/";
}

//============================================================
//  osd_subst_env
//============================================================
void osd_subst_env(char **dst, const char *src)
{
	int i, j;
	char *envstr;

	osd_free(*dst);
	*dst = (char *) osd_malloc_array(strlen(src)+1);
	strcpy(*dst, src);

	// start with an environment variable?
	if (*dst[0] == '$')
	{
		envstr = (char *) osd_malloc_array(strlen(src)+1);
		strcpy(envstr, src);

		i = 0;
		while (!osd_is_path_separator(envstr[i]) && envstr[i] != 0 && envstr[i] != '.')
		{
			i++;
		}

		envstr[i] = '\0';

		const char *envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
			j = strlen(envval) + strlen(*dst) + 1;
			osd_free(*dst);
			*dst = (char *) osd_malloc_array(j);

			// start with the value of $HOME
			strcpy(*dst, envval);
			// replace the null with a path separator again
			envstr[i] = PATHSEPCH;
			// append it
			strcat(*dst, &envstr[i]);
		}
		else
			fprintf(stderr, "Warning: osd_subst_env variable %s not found.\n", envstr);
		osd_free(envstr);
	}
}
#endif
