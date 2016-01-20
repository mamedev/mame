// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minifile.c - Minimal core file access functions
//
//============================================================

#include "osdcore.h"
#include <stdlib.h>
#include <unistd.h>


//============================================================
//  osd_open
//============================================================

file_error osd_open(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	const char *mode;
	FILE *fileptr;

	// based on the flags, choose a mode
	if (openflags & OPEN_FLAG_WRITE)
	{
		if (openflags & OPEN_FLAG_READ)
			mode = (openflags & OPEN_FLAG_CREATE) ? "w+b" : "r+b";
		else
			mode = "wb";
	}
	else if (openflags & OPEN_FLAG_READ)
		mode = "rb";
	else
		return FILERR_INVALID_ACCESS;

	// open the file
	fileptr = fopen(path, mode);
	if (fileptr == nullptr)
		return FILERR_NOT_FOUND;

	// store the file pointer directly as an osd_file
	*file = (osd_file *)fileptr;

	// get the size -- note that most fseek/ftell implementations are limited to 32 bits
	fseek(fileptr, 0, SEEK_END);
	*filesize = ftell(fileptr);
	fseek(fileptr, 0, SEEK_SET);

	return FILERR_NONE;
}


//============================================================
//  osd_close
//============================================================

file_error osd_close(osd_file *file)
{
	// close the file handle
	fclose((FILE *)file);
	return FILERR_NONE;
}


//============================================================
//  osd_read
//============================================================

file_error osd_read(osd_file *file, void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	size_t count;

	// seek to the new location; note that most fseek implementations are limited to 32 bits
	fseek((FILE *)file, offset, SEEK_SET);

	// perform the read
	count = fread(buffer, 1, length, (FILE *)file);
	if (actual != nullptr)
		*actual = count;

	return FILERR_NONE;
}


//============================================================
//  osd_write
//============================================================

file_error osd_write(osd_file *file, const void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	size_t count;

	// seek to the new location; note that most fseek implementations are limited to 32 bits
	fseek((FILE *)file, offset, SEEK_SET);

	// perform the write
	count = fwrite(buffer, 1, length, (FILE *)file);
	if (actual != nullptr)
		*actual = count;

	return FILERR_NONE;
}

//============================================================
//  osd_openpty
//============================================================

file_error osd_openpty(osd_file **file, char *name, size_t name_len)
{
	return FILERR_FAILURE;
}

//============================================================
//  osd_truncate
//============================================================

file_error osd_truncate(osd_file *file, UINT64 offset)
{
	UINT32 result;

	if (!file)
		return FILERR_FAILURE;

	result = ftruncate(fileno((FILE *)file), offset);
	if (result)
		return FILERR_FAILURE;

	return FILERR_NONE;
}


//============================================================
//  osd_rmfile
//============================================================

file_error osd_rmfile(const char *filename)
{
	return remove(filename) ? FILERR_FAILURE : FILERR_NONE;
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	// there is no standard way of doing this, so we always return FALSE, indicating
	// that a given path is not a physical drive
	return FALSE;
}


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(UINT32 /* unicode_char */ *uchar, const char *osdchar, size_t count)
{
	// we assume a standard 1:1 mapping of characters to the first 256 unicode characters
	*uchar = (UINT8)*osdchar;
	return 1;
}


//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	osd_directory_entry *result = nullptr;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *)osd_malloc_array(sizeof(*result) + strlen(path) + 1);
	strcpy((char *)(result + 1), path);
	result->name = (char *)(result + 1);
	result->type = ENTTYPE_NONE;
	result->size = 0;

	FILE *f = fopen(path, "rb");
	if (f != nullptr)
	{
		fseek(f, 0, SEEK_END);
		result->type = ENTTYPE_FILE;
		result->size = ftell(f);
		fclose(f);
	}
	return result;
}


//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	// derive the full path of the file in an allocated string
	// for now just fake it since we don't presume any underlying file system
	*dst = nullptr;
	if (path != nullptr)
	{
		*dst = (char *)osd_malloc_array(strlen(path) + 1);
		if (*dst != nullptr)
			strcpy(*dst, path);
	}

	return FILERR_NONE;
}


//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	// we don't expose volumes
	return nullptr;
}
