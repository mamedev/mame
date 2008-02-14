//============================================================
//
//  winfile.c - Win32 OSD core file access functions
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct _osd_file
{
	HANDLE		handle;
	TCHAR 		filename[1];
};



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static DWORD create_path_recursive(const TCHAR *path);



//============================================================
//  osd_open
//============================================================

file_error osd_open(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	DWORD disposition, access, sharemode;
	file_error filerr = FILERR_NONE;
	const TCHAR *src;
	DWORD upper;
	TCHAR *t_path;
	TCHAR *dst;

	// convert path to TCHAR
	t_path = tstring_from_utf8(path);
	if (t_path == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto error;
	}

	// allocate a file object, plus space for the converted filename
	*file = malloc(sizeof(**file) + sizeof(TCHAR) * _tcslen(t_path));
	if (*file == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto error;
	}

	// convert the path into something Windows compatible
	dst = (*file)->filename;
	for (src = t_path; *src != 0; src++)
		*dst++ = *src;//(*src == '/') ? '\\' : *src;
	*dst++ = 0;

	// select the file open modes
	if (openflags & OPEN_FLAG_WRITE)
	{
		disposition = (openflags & OPEN_FLAG_CREATE) ? CREATE_ALWAYS : OPEN_EXISTING;
		access = (openflags & OPEN_FLAG_READ) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_WRITE;
		sharemode = 0;
	}
	else if (openflags & OPEN_FLAG_READ)
	{
		disposition = OPEN_EXISTING;
		access = GENERIC_READ;
		sharemode = FILE_SHARE_READ;
	}
	else
	{
		filerr = FILERR_INVALID_ACCESS;
		goto error;
	}

	// attempt to open the file
	(*file)->handle = CreateFile((*file)->filename, access, sharemode, NULL, disposition, 0, NULL);
	if ((*file)->handle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();

		// create the path if necessary
		if (error == ERROR_PATH_NOT_FOUND && (openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			TCHAR *pathsep = _tcsrchr((*file)->filename, '\\');
			if (pathsep != NULL)
			{
				// create the path up to the file
				*pathsep = 0;
				error = create_path_recursive((*file)->filename);
				*pathsep = '\\';

				// attempt to reopen the file
				if (error == NO_ERROR)
					(*file)->handle = CreateFile((*file)->filename, access, sharemode, NULL, disposition, 0, NULL);
			}
		}

		// if we still failed, clean up and free
		if ((*file)->handle == INVALID_HANDLE_VALUE)
		{
			filerr = win_error_to_file_error(error);
			goto error;
		}
	}

	// get the file size
	*filesize = GetFileSize((*file)->handle, &upper);
	*filesize |= (UINT64)upper << 32;

error:
	// cleanup
	if (filerr != FILERR_NONE && *file != NULL)
	{
		free(*file);
		*file = NULL;
	}
	free(t_path);
	return filerr;
}


//============================================================
//  osd_read
//============================================================

file_error osd_read(osd_file *file, void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	LONG upper = offset >> 32;
	DWORD result;

	// attempt to set the file pointer
	result = SetFilePointer(file->handle, (UINT32)offset, &upper, FILE_BEGIN);
	if (result == INVALID_SET_FILE_POINTER)
	{
		DWORD error = GetLastError();
		if (error != NO_ERROR)
			return win_error_to_file_error(error);
	}

	// then perform the read
	if (!ReadFile(file->handle, buffer, length, &result, NULL))
		return win_error_to_file_error(GetLastError());
	if (actual != NULL)
		*actual = result;
	return FILERR_NONE;
}


//============================================================
//  osd_write
//============================================================

file_error osd_write(osd_file *file, const void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	LONG upper = offset >> 32;
	DWORD result;

	// attempt to set the file pointer
	result = SetFilePointer(file->handle, (UINT32)offset, &upper, FILE_BEGIN);
	if (result == INVALID_SET_FILE_POINTER)
	{
		DWORD error = GetLastError();
		if (error != NO_ERROR)
			return win_error_to_file_error(error);
	}

	// then perform the read
	if (!WriteFile(file->handle, buffer, length, &result, NULL))
		return win_error_to_file_error(GetLastError());
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
	CloseHandle(file->handle);
	free(file);
	return FILERR_NONE;
}


//============================================================
//  osd_rmfile
//============================================================

file_error osd_rmfile(const char *filename)
{
	file_error filerr = FILERR_NONE;

	TCHAR *tempstr = tstring_from_utf8(filename);
	if (!tempstr)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto done;
	}

	if (!DeleteFile(tempstr))
	{
		filerr = win_error_to_file_error(GetLastError());
		goto done;
	}

done:
	if (tempstr)
		free(tempstr);
	return filerr;
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	DISK_GEOMETRY dg;
	DWORD bytesRead;
	TCHAR *t_filename;
	HANDLE file;
	int result;

	// if it doesn't smell like a physical drive, just return FALSE
	if (_strnicmp(filename, "\\\\.\\physicaldrive", 17) != 0)
		return FALSE;

	// do a create file on the drive
	t_filename = tstring_from_utf8(filename);
	if (t_filename == NULL)
		return FALSE;
	file = CreateFile(t_filename, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	free(t_filename);
	if (file == INVALID_HANDLE_VALUE)
		return FALSE;

	// device I/O control should return the geometry
	result = DeviceIoControl(file, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &bytesRead, NULL);
	CloseHandle(file);

	// if that failed, return false
	if (!result)
		return FALSE;

	// store the results
  	*cylinders = (UINT32)dg.Cylinders.QuadPart;
  	*heads = dg.TracksPerCylinder;
  	*sectors = dg.SectorsPerTrack;
  	*bps = dg.BytesPerSector;

  	// normalize
  	while (*heads > 16 && !(*heads & 1))
  	{
  		*heads /= 2;
  		*cylinders *= 2;
  	}
  	return TRUE;
}


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(UINT32 *uchar, const char *osdchar, size_t count)
{
	WCHAR wch;

	count = MIN(count, IsDBCSLeadByte(*osdchar) ? 2 : 1);
	if (MultiByteToWideChar(CP_ACP, 0, osdchar, (DWORD)count, &wch, 1) != 0)
		*uchar = wch;
	else
		*uchar = 0;
	return (int) count;
}


//============================================================
//  create_path_recursive
//============================================================

DWORD create_path_recursive(const TCHAR *path)
{
	TCHAR *sep = _tcsrchr(path, '\\');
	file_error filerr;

	// if there's still a separator, and it's not the root, nuke it and recurse
	if (sep != NULL && sep > path && sep[0] != ':' && sep[-1] != '\\')
	{
		*sep = 0;
		filerr = create_path_recursive(path);
		*sep = '\\';
	}

	// if the path already exists, we're done
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
		return NO_ERROR;

	// create the path
	if (CreateDirectory(path, NULL) == 0)
		return GetLastError();
	return NO_ERROR;
}
