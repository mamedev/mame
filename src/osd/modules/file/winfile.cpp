// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winfile.c - Win32 OSD core file access functions
//
//============================================================

#define WIN32_LEAN_AND_MEAN

#include "winfile.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"
#include "winutf8.h"
#include "unicode.h"

// MAME headers
#include "osdcore.h"

#include <cassert>
#include <cstring>

// standard windows headers
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <ctype.h>


namespace {
//============================================================
//  TYPE DEFINITIONS
//============================================================

class win_osd_file : public osd_file
{
public:
	win_osd_file(win_osd_file const &) = delete;
	win_osd_file(win_osd_file &&) = delete;
	win_osd_file& operator=(win_osd_file const &) = delete;
	win_osd_file& operator=(win_osd_file &&) = delete;

	win_osd_file(HANDLE handle) : m_handle(handle)
	{
		assert(m_handle);
		assert(INVALID_HANDLE_VALUE != m_handle);
	}

	virtual ~win_osd_file() override
	{
		FlushFileBuffers(m_handle);
		CloseHandle(m_handle);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_file_error(GetLastError());

		// then perform the read
		DWORD result = 0;
		if (!ReadFile(m_handle, buffer, length, &result, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = result;
		return error::NONE;
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_file_error(GetLastError());

		// then perform the write
		DWORD result = 0;
		if (!WriteFile(m_handle, buffer, length, &result, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = result;
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_file_error(GetLastError());

		// then perform the truncation
		if (!SetEndOfFile(m_handle))
			return win_error_to_file_error(GetLastError());
		else
			return error::NONE;
	}

	virtual error flush() override
	{
		// shouldn't be any userspace buffers on the file handle
		return error::NONE;
	}

private:
	HANDLE m_handle;
};


template <typename T>
class osd_disposer
{
public:
	osd_disposer(T *&ptr) : m_ptr(ptr) { }
	~osd_disposer() { if (m_ptr) osd_free(m_ptr); }
private:
	T *&m_ptr;
};



//============================================================
//  INLINE FUNCTIONS
//============================================================

inline bool is_path_to_physical_drive(char const *path)
{
	return (_strnicmp(path, "\\\\.\\physicaldrive", 17) == 0);
}



//============================================================
//  create_path_recursive
//============================================================

DWORD create_path_recursive(TCHAR *path)
{
	// if there's still a separator, and it's not the root, nuke it and recurse
	TCHAR *sep = _tcsrchr(path, '\\');
	if (sep && (sep > path) && (sep[0] != ':') && (sep[-1] != '\\'))
	{
		*sep = 0;
		create_path_recursive(path);
		*sep = '\\';
	}

	// if the path already exists, we're done
	WIN32_FILE_ATTRIBUTE_DATA fileinfo;
	if (GetFileAttributesEx(path, GetFileExInfoStandard, &fileinfo))
		return NO_ERROR;
	else if (!CreateDirectory(path, nullptr))
		return GetLastError();
	else
		return NO_ERROR;
}

} // anonymous namespace



//============================================================
//  osd_open
//============================================================

osd_file::error osd_file::open(std::string const &orig_path, UINT32 openflags, ptr &file, std::uint64_t &filesize)
{
	std::string path;
	try { osd_subst_env(path, orig_path); }
	catch (...) { return error::OUT_OF_MEMORY; }

	if (win_check_socket_path(path))
		return win_open_socket(path, openflags, file, filesize);
	else if (win_check_ptty_path(path))
		return win_open_ptty(path, openflags, file, filesize);

	// convert path to TCHAR
	auto t_path = tstring_from_utf8(path.c_str());

	// select the file open modes
	DWORD disposition, access, sharemode;
	if (openflags & OPEN_FLAG_WRITE)
	{
		disposition = (!is_path_to_physical_drive(path.c_str()) && (openflags & OPEN_FLAG_CREATE)) ? CREATE_ALWAYS : OPEN_EXISTING;
		access = (openflags & OPEN_FLAG_READ) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_WRITE;
		sharemode = FILE_SHARE_READ;
	}
	else if (openflags & OPEN_FLAG_READ)
	{
		disposition = OPEN_EXISTING;
		access = GENERIC_READ;
		sharemode = FILE_SHARE_READ;
	}
	else
	{
		return error::INVALID_ACCESS;
	}

	// attempt to open the file
	HANDLE h = CreateFile(t_path.c_str(), access, sharemode, nullptr, disposition, 0, nullptr);
	if (INVALID_HANDLE_VALUE == h)
	{
		DWORD err = GetLastError();
		// create the path if necessary
		if ((ERROR_PATH_NOT_FOUND == err) && (openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			TCHAR *pathsep = _tcsrchr(&t_path[0], '\\');
			if (pathsep != nullptr)
			{
				// create the path up to the file
				*pathsep = 0;
				err = create_path_recursive(&t_path[0]);
				*pathsep = '\\';

				// attempt to reopen the file
				if (err == NO_ERROR)
				{
					h = CreateFile(t_path.c_str(), access, sharemode, nullptr, disposition, 0, nullptr);
					err = GetLastError();
				}
			}
		}

		// if we still failed, clean up and free
		if (INVALID_HANDLE_VALUE == h)
			return win_error_to_file_error(err);
	}

	// get the file size
	DWORD upper, lower;
	lower = GetFileSize(h, &upper);
	if (INVALID_FILE_SIZE == lower)
	{
		DWORD const err = GetLastError();
		if (NO_ERROR != err)
		{
			CloseHandle(h);
			return win_error_to_file_error(err);
		}
	}

	try
	{
		file = std::make_unique<win_osd_file>(h);
		filesize = (std::uint64_t(upper) << 32) | lower;
		return error::NONE;
	}
	catch (...)
	{
		CloseHandle(h);
		return error::OUT_OF_MEMORY;
	}
}



//============================================================
//  osd_openpty
//============================================================

osd_file::error osd_file::openpty(ptr &file, std::string &name)
{
	return error::FAILURE;
}



//============================================================
//  osd_rmfile
//============================================================

osd_file::error osd_file::remove(std::string const &filename)
{
	auto tempstr = tstring_from_utf8(filename.c_str());

	error filerr = error::NONE;
	if (!DeleteFile(tempstr.c_str()))
		filerr = win_error_to_file_error(GetLastError());

	return filerr;
}



//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	DISK_GEOMETRY dg;
	DWORD bytesRead;
	HANDLE file;
	int result;

	// if it doesn't smell like a physical drive, just return FALSE
	if (!is_path_to_physical_drive(filename))
		return FALSE;

	// do a create file on the drive
	auto t_filename = tstring_from_utf8(filename);
	file = CreateFile(t_filename.c_str(), GENERIC_READ, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return FALSE;

	// device I/O control should return the geometry
	result = DeviceIoControl(file, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &dg, sizeof(dg), &bytesRead, nullptr);
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
//  osd_stat
//============================================================

std::unique_ptr<osd::directory::entry> osd_stat(const std::string &path)
{
	// convert the path to TCHARs
	auto t_path = tstring_from_utf8(path.c_str());

	// is this path a root directory (e.g. - C:)?
	WIN32_FIND_DATA find_data;
	std::memset(&find_data, 0, sizeof(find_data));
	if (isalpha(path[0]) && (path[1] == ':') && (path[2] == '\0'))
	{
		// need to do special logic for root directories
		if (!GetFileAttributesEx(t_path.c_str(), GetFileExInfoStandard, &find_data.dwFileAttributes))
			find_data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
	}
	else
	{
		// attempt to find the first file
		HANDLE find = FindFirstFileEx(t_path.c_str(), FindExInfoStandard, &find_data, FindExSearchNameMatch, nullptr, 0);
		if (find == INVALID_HANDLE_VALUE)
			return nullptr;
		FindClose(find);
	}

	// create an osd::directory::entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd::directory::entry
	osd::directory::entry *result;
	try { result = reinterpret_cast<osd::directory::entry *>(::operator new(sizeof(*result) + path.length() + 1)); }
	catch (...) { return nullptr; }
	new (result) osd::directory::entry;

	strcpy(((char *) result) + sizeof(*result), path.c_str());
	result->name = ((char *) result) + sizeof(*result);
	result->type = win_attributes_to_entry_type(find_data.dwFileAttributes);
	result->size = find_data.nFileSizeLow | ((UINT64) find_data.nFileSizeHigh << 32);
	result->last_modified = win_time_point_from_filetime(&find_data.ftLastWriteTime);

	return std::unique_ptr<osd::directory::entry>(result);
}


//============================================================
//  osd_get_full_path
//============================================================

osd_file::error osd_get_full_path(std::string &dst, std::string const &path)
{
	// convert the path to TCHARs
	auto t_path = tstring_from_utf8(path.c_str());

	// cannonicalize the path
	TCHAR buffer[MAX_PATH];
	if (!GetFullPathName(t_path.c_str(), ARRAY_LENGTH(buffer), buffer, nullptr))
		return win_error_to_file_error(GetLastError());

	// convert the result back to UTF-8
	utf8_from_tstring(dst, buffer);
	return osd_file::error::NONE;
}



//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path)
{
	auto t_path = tstring_from_utf8(path.c_str());
	return !PathIsRelative(t_path.c_str());
}



//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	static char szBuffer[128];
	const char *p;

	GetLogicalDriveStringsA(ARRAY_LENGTH(szBuffer), szBuffer);

	p = szBuffer;
	while(idx--) {
		p += strlen(p) + 1;
		if (!*p) return nullptr;
	}

	return p;
}



//============================================================
//  osd_is_valid_filename_char
//============================================================

bool osd_is_valid_filename_char(unicode_char uchar)
{
	return osd_is_valid_filepath_char(uchar)
		&& uchar != '/'
		&& uchar != '\\'
		&& uchar != ':';
}



//============================================================
//  osd_is_valid_filepath_char
//============================================================

bool osd_is_valid_filepath_char(unicode_char uchar)
{
	return uchar >= 0x20
		&& uchar != '<'
		&& uchar != '>'
		&& uchar != '\"'
		&& uchar != '|'
		&& uchar != '?'
		&& uchar != '*'
		&& !(uchar >= '\x7F' && uchar <= '\x9F')
		&& uchar_isvalid(uchar);
}



//============================================================
//  win_error_to_file_error
//============================================================

osd_file::error win_error_to_file_error(DWORD error)
{
	osd_file::error filerr;

	// convert a Windows error to a osd_file::error
	switch (error)
	{
	case ERROR_SUCCESS:
		filerr = osd_file::error::NONE;
		break;

	case ERROR_OUTOFMEMORY:
		filerr = osd_file::error::OUT_OF_MEMORY;
		break;

	case ERROR_FILE_NOT_FOUND:
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_PATH_NOT_FOUND:
		filerr = osd_file::error::NOT_FOUND;
		break;

	case ERROR_ACCESS_DENIED:
		filerr = osd_file::error::ACCESS_DENIED;
		break;

	case ERROR_SHARING_VIOLATION:
		filerr = osd_file::error::ALREADY_OPEN;
		break;

	default:
		filerr = osd_file::error::FAILURE;
		break;
	}
	return filerr;
}
