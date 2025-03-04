// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlfile.c - SDL file access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================


#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef __linux__
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef __USE_BSD
#define __USE_BSD
#endif
#endif

#ifdef _WIN32
#define _FILE_OFFSET_BITS 64
#endif

#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && !defined(__DragonFly__)
#ifdef _XOPEN_SOURCE
#if _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
#endif
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#endif

// Fix for MacOS compilation errors
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif

// MAME headers
#include "posixfile.h"
#include "osdcore.h"
#include "unicode.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include <fcntl.h>
#include <climits>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>



namespace {

//============================================================
//  CONSTANTS
//============================================================

#if defined(_WIN32)
constexpr char PATHSEPCH = '\\';
constexpr char INVPATHSEPCH = '/';
#else
constexpr char PATHSEPCH = '/';
#endif



class posix_osd_file : public osd_file
{
public:
	posix_osd_file(posix_osd_file const &) = delete;
	posix_osd_file(posix_osd_file &&) = delete;
	posix_osd_file& operator=(posix_osd_file const &) = delete;
	posix_osd_file& operator=(posix_osd_file &&) = delete;

	posix_osd_file(int fd) noexcept : m_fd(fd)
	{
		assert(m_fd >= 0);
	}

	virtual ~posix_osd_file() override
	{
		::close(m_fd);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		ssize_t result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__)
		result = ::pread(m_fd, buffer, size_t(count), off_t(std::make_unsigned_t<off_t>(offset)));
#elif defined(_WIN32) || defined(SDLMAME_NO64BITIO)
		if (lseek(m_fd, off_t(std::make_unsigned_t<off_t>(offset)), SEEK_SET) < 0)
			return std::error_condition(errno, std::generic_category());
		result = ::read(m_fd, buffer, size_t(count));
#else
		result = ::pread64(m_fd, buffer, size_t(count), off64_t(offset));
#endif

		if (result < 0)
			return std::error_condition(errno, std::generic_category());

		actual = std::uint32_t(std::size_t(result));
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		ssize_t result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__)
		result = ::pwrite(m_fd, buffer, size_t(count), off_t(std::make_unsigned_t<off_t>(offset)));
#elif defined(_WIN32) || defined(SDLMAME_NO64BITIO)
		if (lseek(m_fd, off_t(std::make_unsigned_t<off_t>(offset)), SEEK_SET) < 0)
			return std::error_condition(errno, std::generic_category());
		result = ::write(m_fd, buffer, size_t(count));
#else
		result = ::pwrite64(m_fd, buffer, size_t(count), off64_t(offset));
#endif

		if (result < 0)
			return std::error_condition(errno, std::generic_category());

		actual = std::uint32_t(std::size_t(result));
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		int result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__EMSCRIPTEN__) || defined(_WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
		result = ::ftruncate(m_fd, off_t(std::make_unsigned_t<off_t>(offset)));
#else
		result = ::ftruncate64(m_fd, off64_t(offset));
#endif

		if (result < 0)
			return std::error_condition(errno, std::generic_category());

		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		// no user-space buffering on unistd I/O
		return std::error_condition();
	}

private:
	int m_fd;
};


//============================================================
//  is_path_separator
//============================================================

bool is_path_separator(char c) noexcept
{
#if defined(_WIN32)
	return (c == PATHSEPCH) || (c == INVPATHSEPCH);
#else
	return c == PATHSEPCH;
#endif
}


//============================================================
//  create_path_recursive
//============================================================

std::error_condition create_path_recursive(std::string_view path) noexcept
{
	// if there's still a separator, and it's not the root, nuke it and recurse
	auto const sep = path.rfind(PATHSEPCH);
	if ((sep != std::string_view::npos) && (sep > 0) && (path[sep - 1] != PATHSEPCH))
	{
		std::error_condition err = create_path_recursive(path.substr(0, sep));
		if (err)
			return err;
	}

	// need a NUL-terminated version of the subpath
	std::string p;
	try { p = path; }
	catch (...) { return std::errc::not_enough_memory; }

	// if the path already exists, we're done
	struct stat st;
	if (!::stat(p.c_str(), &st))
		return std::error_condition();

	// create the path
#ifdef _WIN32
	if (mkdir(p.c_str()) < 0)
#else
	if (mkdir(p.c_str(), 0777) < 0)
#endif
		return std::error_condition(errno, std::generic_category());
	else
		return std::error_condition();
}

} // anonymous namespace


//============================================================
//  osd_file::open
//============================================================

std::error_condition osd_file::open(std::string const &path, std::uint32_t openflags, ptr &file, std::uint64_t &filesize) noexcept
{
	std::string dst;
	if (posix_check_socket_path(path))
		return posix_open_socket(path, openflags, file, filesize);
	else if (posix_check_ptty_path(path))
		return posix_open_ptty(openflags, file, filesize, dst);
	else if (posix_check_domain_path(path))
		return posix_open_domain(path, openflags, file, filesize);

	// select the file open modes
	int access;
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
		return std::errc::invalid_argument;
	}
#if defined(_WIN32)
	access |= O_BINARY;
#endif

	// convert the path into something compatible
	dst = path;
#if defined(_WIN32)
	for (auto it = dst.begin(); it != dst.end(); ++it)
		*it = (INVPATHSEPCH == *it) ? PATHSEPCH : *it;
#endif

	// attempt to open the file
	int fd = -1;
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(_WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	fd = ::open(dst.c_str(), access, 0666);
#else
	fd = ::open64(dst.c_str(), access, 0666);
#endif

	if (fd < 0)
	{
		// save the error from the first attempt to open the file
		std::error_condition openerr(errno, std::generic_category());

		// create the path if necessary
		if ((openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			auto const pathsep = dst.rfind(PATHSEPCH);
			if (pathsep != std::string::npos)
			{
				// create the path up to the file
				std::error_condition const createrr = create_path_recursive(dst.substr(0, pathsep));

				// attempt to reopen the file
				if (!createrr)
				{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(_WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
					fd = ::open(dst.c_str(), access, 0666);
#else
					fd = ::open64(dst.c_str(), access, 0666);
#endif
				}
				if (fd < 0)
				{
					openerr.assign(errno, std::generic_category());
				}
			}
		}

		// if we still failed, clean up and free
		if (fd < 0)
		{
			return openerr;
		}
	}

	// get the file size
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(_WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	struct stat st;
	if (::fstat(fd, &st) < 0)
#else
	struct stat64 st;
	if (::fstat64(fd, &st) < 0)
#endif
	{
		std::error_condition staterr(errno, std::generic_category());
		::close(fd);
		return staterr;
	}

	osd_file::ptr result(new (std::nothrow) posix_osd_file(fd));
	if (!result)
	{
		::close(fd);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = std::uint64_t(std::make_unsigned_t<decltype(st.st_size)>(st.st_size));
	return std::error_condition();
}


//============================================================
//  osd_file::openpty
//============================================================

std::error_condition osd_file::openpty(ptr &file, std::string &name) noexcept
{
	std::uint64_t filesize;
	return posix_open_ptty(OPEN_FLAG_READ | OPEN_FLAG_WRITE, file, filesize, name);
}


//============================================================
//  osd_file::remove
//============================================================

std::error_condition osd_file::remove(std::string const &filename) noexcept
{
	if (::unlink(filename.c_str()) < -1)
		return std::error_condition(errno, std::generic_category());
	else
		return std::error_condition();
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps) noexcept
{
	return false; // no, no way, huh-uh, forget it
}


//============================================================
//  osd_stat
//============================================================

std::unique_ptr<osd::directory::entry> osd_stat(const std::string &path)
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(_WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	struct stat st;
	int const err = ::stat(path.c_str(), &st);
#else
	struct stat64 st;
	int const err = ::stat64(path.c_str(), &st);
#endif
	if (err < 0) return nullptr;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	auto const result = reinterpret_cast<osd::directory::entry *>(
			::operator new(
				sizeof(osd::directory::entry) + path.length() + 1,
				std::align_val_t(alignof(osd::directory::entry)),
				std::nothrow));
	if (!result) return nullptr;
	new (result) osd::directory::entry;

	auto const resultname = reinterpret_cast<char *>(result) + sizeof(*result);
	std::strcpy(resultname, path.c_str());
	result->name = resultname;
	result->type = S_ISDIR(st.st_mode) ? osd::directory::entry::entry_type::DIR : osd::directory::entry::entry_type::FILE;
	result->size = std::uint64_t(std::make_unsigned_t<decltype(st.st_size)>(st.st_size));
	result->last_modified = std::chrono::system_clock::from_time_t(st.st_mtime);

	return std::unique_ptr<osd::directory::entry>(result);
}


//============================================================
//  osd_get_full_path
//============================================================

std::error_condition osd_get_full_path(std::string &dst, std::string const &path) noexcept
{
	try
	{
#if defined(_WIN32)
		std::vector<char> path_buffer(MAX_PATH);
		if (::_fullpath(&path_buffer[0], path.c_str(), MAX_PATH))
		{
			dst = &path_buffer[0];
			return std::error_condition();
		}
		else
		{
			return std::errc::io_error; // TODO: better error reporting?
		}
#else
		std::unique_ptr<char, void (*)(void *)> canonical(::realpath(path.c_str(), nullptr), &std::free);
		if (canonical)
		{
			dst = canonical.get();
			return std::error_condition();
		}

		std::vector<char> path_buffer(PATH_MAX);
		if (::realpath(path.c_str(), &path_buffer[0]))
		{
			dst = &path_buffer[0];
			return std::error_condition();
		}
		else if (path[0] == PATHSEPCH)
		{
			dst = path;
			return std::error_condition();
		}
		else
		{
			while (!::getcwd(&path_buffer[0], path_buffer.size()))
			{
				if (errno != ERANGE)
					return std::error_condition(errno, std::generic_category());
				else
					path_buffer.resize(path_buffer.size() * 2);
			}
			dst.assign(&path_buffer[0]).push_back(PATHSEPCH);
			dst.append(path);
			return std::error_condition();
		}
#endif
	}
	catch (...)
	{
		return std::errc::not_enough_memory;
	}
}


//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path) noexcept
{
	if (!path.empty() && is_path_separator(path[0]))
		return true;
#if !defined(_WIN32)
	else if (!path.empty() && (path[0] == '.') && (!path[1] || is_path_separator(path[1]))) // FIXME: why is this even here? foo/./bar is a valid way to refer to foo/bar
		return true;
#elif !defined(UNDER_CE)
	else if ((path.length() > 1) && (path[1] == ':'))
		return true;
#endif
	else
		return false;
}


//============================================================
//  osd_get_volume_name
//============================================================

std::string osd_get_volume_name(int idx)
{
	if (idx == 0)
		return "/";
	else
		return std::string();
}


//============================================================
//  osd_get_volume_names
//============================================================

std::vector<std::string> osd_get_volume_names()
{
	return std::vector<std::string>{ "/" };
}


//============================================================
//  osd_is_valid_filename_char
//============================================================

bool osd_is_valid_filename_char(char32_t uchar) noexcept
{
	// The only one that's actually invalid is the slash
	// The other two are just problematic because they're the escape character and path separator
	return osd_is_valid_filepath_char(uchar)
#if defined(_WIN32)
		&& uchar != PATHSEPCH
		&& uchar != INVPATHSEPCH
#else
		&& uchar != PATHSEPCH
		&& uchar != '\\'
#endif
		&& uchar != ':';
}


//============================================================
//  osd_is_valid_filepath_char
//============================================================

bool osd_is_valid_filepath_char(char32_t uchar) noexcept
{
	// One could argue that colon should be in here too because it functions as path separator
	return uchar >= 0x20
		&& !(uchar >= '\x7F' && uchar <= '\x9F')
#if defined(_WIN32)
		&& uchar != '<'
		&& uchar != '>'
		&& uchar != '\"'
		&& uchar != '|'
		&& uchar != '?'
		&& uchar != '*'
#endif
		&& uchar_isvalid(uchar);
}
