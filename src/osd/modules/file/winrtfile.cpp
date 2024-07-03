// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winfile.c - Win32 OSD core file access functions
//
//============================================================


#include "winfile.h"

#include "../../windows/winutil.h"

// MAMEOS headers
#include "strconv.h"
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
#include <cstdlib>
#include <cctype>


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

osd_file::error osd_file::open(std::string const &path, uint32_t openflags, ptr &file, std::uint64_t &filesize)
{
	if (win_check_socket_path(path))
		return win_open_socket(path, openflags, file, filesize);
	else if (win_check_ptty_path(path))
		return win_open_ptty(path, openflags, file, filesize);

	// convert path to TCHAR
	osd::text::tstring t_path = osd::text::to_tstring(path);

	// convert the path into something Windows compatible (the actual interesting part appears
	// to have been commented out???)
	for (auto iter = t_path.begin(); iter != t_path.end(); iter++)
		*iter = /* ('/' == *iter) ? '\\' : */ *iter;

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
	HANDLE h = CreateFile2(t_path.c_str(), access, sharemode, disposition, nullptr);
	if (INVALID_HANDLE_VALUE == h)
	{
		DWORD err = GetLastError();
		// create the path if necessary
		if ((ERROR_PATH_NOT_FOUND == err) && (openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			auto pathsep = t_path.rfind('\\');
			if (pathsep != decltype(t_path)::npos)
			{
				// create the path up to the file
				t_path[pathsep] = 0;
				err = create_path_recursive(&t_path[0]);
				t_path[pathsep] = '\\';

				// attempt to reopen the file
				if (err == NO_ERROR)
				{
					h = CreateFile2(t_path.c_str(), access, sharemode, disposition, nullptr);
					err = GetLastError();
				}
			}
		}

		// if we still failed, clean up and free
		if (INVALID_HANDLE_VALUE == h)
			return win_error_to_file_error(err);
	}

	// get the file size
	FILE_STANDARD_INFO file_info;
	GetFileInformationByHandleEx(h, FileStandardInfo, &file_info, sizeof(file_info));
	if (INVALID_FILE_SIZE == file_info.EndOfFile.LowPart)
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
		filesize = file_info.EndOfFile.QuadPart;
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
	osd::text::tstring tempstr = osd::text::to_tstring(filename);

	error filerr = error::NONE;
	if (!DeleteFile(tempstr.c_str()))
		filerr = win_error_to_file_error(GetLastError());

	return filerr;
}



//============================================================
//  osd_get_physical_drive_geometry
//============================================================

bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps)
{
	return false;
}



//============================================================
//  osd_stat
//============================================================

std::unique_ptr<osd::directory::entry> osd_stat(const std::string &path)
{
	// convert the path to TCHARs
	osd::text::tstring t_path = osd::text::to_tstring(path);

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
	result->size = find_data.nFileSizeLow | ((uint64_t) find_data.nFileSizeHigh << 32);
	result->last_modified = win_time_point_from_filetime(&find_data.ftLastWriteTime);

	return std::unique_ptr<osd::directory::entry>(result);
}


//============================================================
//  osd_get_full_path
//============================================================

osd_file::error osd_get_full_path(std::string &dst, std::string const &path)
{
	if (win_check_socket_path(path) ||
	    win_check_ptty_path(path))
	{
		dst = path;
		return std::error_condition();
	}
	// convert the path to TCHARs
	osd::text::tstring t_path = osd::text::to_tstring(path);

	// canonicalize the path
	TCHAR buffer[MAX_PATH];
	if (!GetFullPathName(t_path.c_str(), std::size(buffer), buffer, nullptr))
		return win_error_to_file_error(GetLastError());

	// convert the result back to UTF-8
	osd::text::from_tstring(dst, buffer);
	return osd_file::error::NONE;
}



//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path)
{
	// very dumb hack here that will be imprecise
	if (strlen(path.c_str()) >= 2 && path[1] == ':')
		return true;

	return false;

}



//============================================================
//  osd_get_volume_name
//============================================================

std::string osd_get_volume_name(int idx)
{
	return std::string();
}


//============================================================
//  osd_get_volume_names
//============================================================

std::vector<std::string> osd_get_volume_names()
{
	return std::vector<std::string>();
}



//============================================================
//  osd_is_valid_filename_char
//============================================================

bool osd_is_valid_filename_char(char32_t uchar)
{
	return osd_is_valid_filepath_char(uchar)
		&& uchar != '/'
		&& uchar != '\\'
		&& uchar != ':';
}



//============================================================
//  osd_is_valid_filepath_char
//============================================================

bool osd_is_valid_filepath_char(char32_t uchar)
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
	case ERROR_INVALID_NAME:
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
