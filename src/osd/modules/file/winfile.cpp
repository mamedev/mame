// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winfile.c - Win32 OSD core file access functions
//
//============================================================

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
#include <memory>

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

	win_osd_file(HANDLE handle) noexcept : m_handle(handle)
	{
		assert(m_handle);
		assert(INVALID_HANDLE_VALUE != m_handle);
	}

	virtual ~win_osd_file() override
	{
		FlushFileBuffers(m_handle);
		CloseHandle(m_handle);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_error_condition(GetLastError());

		// then perform the read
		DWORD result = 0;
		if (!ReadFile(m_handle, buffer, length, &result, nullptr))
			return win_error_to_error_condition(GetLastError());

		actual = result;
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_error_condition(GetLastError());

		// then perform the write
		DWORD result = 0;
		if (!WriteFile(m_handle, buffer, length, &result, nullptr))
			return win_error_to_error_condition(GetLastError());

		actual = result;
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// attempt to set the file pointer
		LARGE_INTEGER largeOffset;
		largeOffset.QuadPart = offset;
		if (!SetFilePointerEx(m_handle, largeOffset, nullptr, FILE_BEGIN))
			return win_error_to_error_condition(GetLastError());

		// then perform the truncation
		if (!SetEndOfFile(m_handle))
			return win_error_to_error_condition(GetLastError());
		else
			return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		// shouldn't be any userspace buffers on the file handle
		return std::error_condition();
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

std::error_condition osd_file::open(std::string const &path, uint32_t openflags, ptr &file, std::uint64_t &filesize) noexcept
{
	if (win_check_socket_path(path))
		return win_open_socket(path, openflags, file, filesize);
	else if (win_check_ptty_path(path))
		return win_open_ptty(path, openflags, file, filesize);

	// convert path to TCHAR
	osd::text::tstring t_path;
	try { t_path = osd::text::to_tstring(path); }
	catch (...) { return std::errc::not_enough_memory; }

	// convert the path into something Windows compatible (the actual interesting part appears to have been commented out???)
	for (auto iter = t_path.begin(); iter != t_path.end(); iter++)
		*iter = /* ('/' == *iter) ? '\\' : */ *iter;

	// select the file open modes
	DWORD disposition, access, sharemode;
	if (openflags & OPEN_FLAG_WRITE)
	{
		disposition = (!is_path_to_physical_drive(path.c_str()) && (openflags & OPEN_FLAG_CREATE)) ? CREATE_ALWAYS : OPEN_EXISTING;
		access = (openflags & OPEN_FLAG_READ) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_WRITE;
		if (is_path_to_physical_drive(path.c_str()))
			access |= GENERIC_READ;
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
		return std::errc::invalid_argument;
	}

	// attempt to open the file
	HANDLE h = CreateFile(t_path.c_str(), access, sharemode, nullptr, disposition, 0, nullptr);
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
					h = CreateFile(t_path.c_str(), access, sharemode, nullptr, disposition, 0, nullptr);
					err = GetLastError();
				}
			}
		}

		// if we still failed, clean up and free
		if (INVALID_HANDLE_VALUE == h)
			return win_error_to_error_condition(err);
	}

	// get the file size
	DWORD upper, lower;
	if (is_path_to_physical_drive(path.c_str()))
	{
		GET_LENGTH_INFORMATION gli;
		DWORD ret;
		if (!DeviceIoControl(h, IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, &gli, sizeof(gli), &ret, nullptr))
		{
			upper = 0;
			lower = INVALID_FILE_SIZE;
		}
		else
		{
			lower = gli.Length.LowPart;
			upper = gli.Length.HighPart;
		}
	}
	else
	{
		lower = GetFileSize(h, &upper);
	}
	if (INVALID_FILE_SIZE == lower)
	{
		DWORD const err = GetLastError();
		if (NO_ERROR != err)
		{
			CloseHandle(h);
			return win_error_to_error_condition(err);
		}
	}

	osd_file::ptr result(new (std::nothrow) win_osd_file(h));
	if (!result)
	{
		CloseHandle(h);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = (std::uint64_t(upper) << 32) | lower;
	return std::error_condition();
}



//============================================================
//  osd_openpty
//============================================================

std::error_condition osd_file::openpty(ptr &file, std::string &name) noexcept
{
	return std::errc::not_supported; // TODO: revisit this error code
}



//============================================================
//  osd_rmfile
//============================================================

std::error_condition osd_file::remove(std::string const &filename) noexcept
{
	osd::text::tstring tempstr;
	try { tempstr = osd::text::to_tstring(filename); }
	catch (...) { return std::errc::not_enough_memory; }

	std::error_condition filerr;
	if (!DeleteFile(tempstr.c_str()))
		filerr = win_error_to_error_condition(GetLastError());

	return filerr;
}



//============================================================
//  osd_get_physical_drive_geometry
//============================================================

bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps) noexcept
{
	DISK_GEOMETRY dg;
	DWORD bytesRead;
	HANDLE file;
	int result;

	// if it doesn't smell like a physical drive, just return false
	if (!is_path_to_physical_drive(filename))
		return false;

	// do a create file on the drive
	try
	{
		auto t_filename = osd::text::to_tstring(filename);
		file = CreateFile(t_filename.c_str(), GENERIC_READ, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return false;
	}
	catch (...)
	{
		return false;
	}

	// device I/O control should return the geometry
	result = DeviceIoControl(file, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &dg, sizeof(dg), &bytesRead, nullptr);
	CloseHandle(file);

	// if that failed, return false
	if (!result)
		return false;

	// store the results
	*cylinders = (uint32_t)dg.Cylinders.QuadPart;
	*heads = dg.TracksPerCylinder;
	*sectors = dg.SectorsPerTrack;
	*bps = dg.BytesPerSector;

	// normalize
	while (*heads > 16 && !(*heads & 1))
	{
		*heads /= 2;
		*cylinders *= 2;
	}
	return true;
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

std::error_condition osd_get_full_path(std::string &dst, std::string const &path) noexcept
{
	try
	{
		if (win_check_socket_path(path) ||
		    win_check_ptty_path(path))
		{
			dst = path;
			return std::error_condition();
		}
		// get the length of the full path
		std::wstring const w_path(osd::text::to_wstring(path));
		DWORD const length(GetFullPathNameW(w_path.c_str(), 0, nullptr, nullptr));
		if (!length)
			return win_error_to_error_condition(GetLastError());

		// allocate a buffer and get the canonical path
		std::unique_ptr<wchar_t []> buffer(std::make_unique<wchar_t []>(length));
		if (!GetFullPathNameW(w_path.c_str(), length, buffer.get(), nullptr))
			return win_error_to_error_condition(GetLastError());

		// convert the result back to UTF-8
		osd::text::from_wstring(dst, buffer.get());
		return std::error_condition();
	}
	catch (...)
	{
		return std::errc::not_enough_memory; // the string conversions can throw bad_alloc
	}
}



//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path) noexcept
{
	return !PathIsRelativeW(osd::text::to_wstring(path).c_str());
}



//============================================================
//  osd_get_volume_name
//============================================================

std::string osd_get_volume_name(int idx)
{
	std::vector<wchar_t> buffer;
	DWORD length(GetLogicalDriveStringsW(0, nullptr));
	while (length && (buffer.size() < (length + 1)))
	{
		buffer.clear();
		buffer.resize(length + 1);
		length = GetLogicalDriveStringsW(length, &buffer[0]);
	}
	if (!length)
		return std::string();

	wchar_t const *p(&buffer[0]);
	while (idx-- && *p)
	{
		while (*p++) { }
	}

	std::string result;
	osd::text::from_wstring(result, p);
	return result;
}


//============================================================
//  osd_get_volume_names
//============================================================

std::vector<std::string> osd_get_volume_names()
{
	std::vector<std::string> result;
	std::vector<wchar_t> buffer;
	DWORD length(GetLogicalDriveStringsW(0, nullptr));
	while (length && (buffer.size() < (length + 1)))
	{
		buffer.clear();
		buffer.resize(length + 1);
		length = GetLogicalDriveStringsW(length, &buffer[0]);
	}
	if (!length)
		return result;

	wchar_t const *p(&buffer[0]);
	std::wstring vol;
	while (*p)
	{
		osd::text::from_wstring(result.emplace_back(), p);
		while (*p++) { }
	}
	return result;
}



//============================================================
//  osd_is_valid_filename_char
//============================================================

bool osd_is_valid_filename_char(char32_t uchar) noexcept
{
	return osd_is_valid_filepath_char(uchar)
		&& uchar != '/'
		&& uchar != '\\'
		&& uchar != ':';
}



//============================================================
//  osd_is_valid_filepath_char
//============================================================

bool osd_is_valid_filepath_char(char32_t uchar) noexcept
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
