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
#define __USE_LARGEFILE64
#endif

#ifdef WIN32
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

// MAME headers
#include "posixfile.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>



namespace {
//============================================================
//  CONSTANTS
//============================================================

#if defined(WIN32)
constexpr char PATHSEPCH = '\\';
constexpr char INVPATHSEPCH = '/';
#else
constexpr char PATHSEPCH = '/';
constexpr char INVPATHSEPCH = '\\';
#endif



class posix_osd_file : public osd_file
{
public:
	posix_osd_file(posix_osd_file const &) = delete;
	posix_osd_file(posix_osd_file &&) = delete;
	posix_osd_file& operator=(posix_osd_file const &) = delete;
	posix_osd_file& operator=(posix_osd_file &&) = delete;

	posix_osd_file(int fd) : m_fd(fd)
	{
		assert(m_fd >= 0);
	}

	virtual ~posix_osd_file() override
	{
		::close(m_fd);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		ssize_t result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(EMSCRIPTEN) || defined(__ANDROID__)
		result = ::pread(m_fd, buffer, size_t(count), off_t(std::make_unsigned_t<off_t>(offset)));
#elif defined(WIN32) || defined(SDLMAME_NO64BITIO)
		if (lseek(m_fd, off_t(std::make_unsigned_t<off_t>(offset)), SEEK_SET) < 0)
			return errno_to_file_error(errno)
		result = ::read(m_fd, buffer, size_t(count));
#else
		result = ::pread64(m_fd, buffer, size_t(count), off64_t(offset));
#endif

		if (result < 0)
			return errno_to_file_error(errno);

		actual = std::uint32_t(std::size_t(result));
		return error::NONE;
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		ssize_t result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(EMSCRIPTEN) || defined(__ANDROID__)
		result = ::pwrite(m_fd, buffer, size_t(count), off_t(std::make_unsigned_t<off_t>(offset)));
#elif defined(WIN32) || defined(SDLMAME_NO64BITIO)
		if (lseek(m_fd, off_t(std::make_unsigned_t<off_t>(offset)), SEEK_SET) < 0)
			return errno_to_file_error(errno)
		result = ::write(m_fd, buffer, size_t(count));
#else
		result = ::pwrite64(m_fd, buffer, size_t(count), off64_t(offset));
#endif

		if (result < 0)
			return errno_to_file_error(errno);

		actual = std::uint32_t(std::size_t(result));
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		int result;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(EMSCRIPTEN) || defined(WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
		result = ::ftruncate(m_fd, off_t(std::make_unsigned_t<off_t>(offset)));
#else
		result = ::ftruncate64(m_fd, off64_t(offset));
#endif

		if (result < 0)
			return errno_to_file_error(errno);

		return error::NONE;
	}

	virtual error flush() override
	{
		// no user-space buffering on unistd I/O
		return error::NONE;
	}

private:
	int m_fd;
};


//============================================================
//  is_path_separator
//============================================================

bool is_path_separator(char c)
{
	return (c == PATHSEPCH) || (c == INVPATHSEPCH);
}


//============================================================
//  create_path_recursive
//============================================================

osd_file::error create_path_recursive(std::string const &path)
{
	// if there's still a separator, and it's not the root, nuke it and recurse
	auto const sep = path.rfind(PATHSEPCH);
	if ((sep != std::string::npos) && (sep > 0) && (path[sep] != ':') && (path[sep - 1] != PATHSEPCH))
	{
		osd_file::error const err = create_path_recursive(path.substr(0, sep));
		if (err != osd_file::error::NONE)
			return err;
	}

	// if the path already exists, we're done
	struct stat st;
	if (!::stat(path.c_str(), &st))
		return osd_file::error::NONE;

	// create the path
#ifdef WIN32
	if (mkdir(path.c_str()) < 0)
#else
	if (mkdir(path.c_str(), 0777) < 0)
#endif
		return errno_to_file_error(errno);
	else
		return osd_file::error::NONE;
}

} // anonymous namespace


//============================================================
//  osd_file::open
//============================================================

osd_file::error osd_file::open(std::string const &path, std::uint32_t openflags, ptr &file, std::uint64_t &filesize)
{
	std::string dst;
	if (posix_check_socket_path(path))
		return posix_open_socket(path, openflags, file, filesize);
	else if (posix_check_ptty_path(path))
		return posix_open_ptty(openflags, file, filesize, dst);

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
		return error::INVALID_ACCESS;
	}
#if defined(WIN32)
	access |= O_BINARY;
#endif

	// convert the path into something compatible
	dst = path;
	for (auto it = dst.begin(); it != dst.end(); ++it)
		*it = (INVPATHSEPCH == *it) ? PATHSEPCH : *it;
	osd_subst_env(dst, dst);

	// attempt to open the file
	int fd = -1;
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	fd = ::open(dst.c_str(), access, 0666);
#else
	fd = ::open64(dst.c_str(), access, 0666);
#endif

	if (fd < 0)
	{
		// create the path if necessary
		if ((openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			auto const pathsep = dst.rfind(PATHSEPCH);
			if (pathsep != std::string::npos)
			{
				// create the path up to the file
				osd_file::error const error = create_path_recursive(dst.substr(0, pathsep));

				// attempt to reopen the file
				if (error == osd_file::error::NONE)
				{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
					fd = ::open(dst.c_str(), access, 0666);
#else
					fd = ::open64(dst.c_str(), access, 0666);
#endif
				}
			}
		}

		// if we still failed, clean up and osd_free
		if (fd < 0)
		{
			return errno_to_file_error(errno);
		}
	}

	// get the file size
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	struct stat st;
	if (::fstat(fd, &st) < 0)
#else
	struct stat64 st;
	if (::fstat64(fd, &st) < 0)
#endif
	{
		int const error = errno;
		::close(fd);
		return errno_to_file_error(error);
	}
	filesize = std::uint64_t(std::make_unsigned_t<decltype(st.st_size)>(st.st_size));

	try
	{
		file = std::make_unique<posix_osd_file>(fd);
		return error::NONE;
	}
	catch (...)
	{
		::close(fd);
		return error::OUT_OF_MEMORY;
	}
}


//============================================================
//  osd_file::openpty
//============================================================

osd_file::error osd_file::openpty(ptr &file, std::string &name)
{
	std::uint64_t   filesize;
	return posix_open_ptty(OPEN_FLAG_READ | OPEN_FLAG_WRITE, file, filesize, name);
}


//============================================================
//  osd_file::remove
//============================================================

osd_file::error osd_file::remove(std::string const &filename)
{
	if (::unlink(filename.c_str()) < -1)
		return errno_to_file_error(errno);
	else
		return error::NONE;
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	return FALSE; // no, no way, huh-uh, forget it
}


//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const std::string &path)
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__HAIKU__) || defined(WIN32) || defined(SDLMAME_NO64BITIO) || defined(__ANDROID__)
	struct stat st;
	int const err = ::stat(path.c_str(), &st);
#else
	struct stat64 st;
	int const err = ::stat64(path.c_str(), &st);
#endif
	if (err < 0) return nullptr;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	osd_directory_entry *const result = reinterpret_cast<osd_directory_entry *>(osd_malloc_array(sizeof(osd_directory_entry) + path.length() + 1));
	std::strcpy(reinterpret_cast<char *>(result) + sizeof(*result), path.c_str());
	result->name = reinterpret_cast<char *>(result) + sizeof(*result);
	result->type = S_ISDIR(st.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	result->size = std::uint64_t(std::make_unsigned_t<decltype(st.st_size)>(st.st_size));

	return result;
}


//============================================================
//  osd_get_full_path
//============================================================

osd_file::error osd_get_full_path(std::string &dst, std::string const &path)
{
	try
	{
#if defined(WIN32)
		std::vector<char> path_buffer(MAX_PATH);
		if (::_fullpath(&path_buffer[0], path.c_str(), MAX_PATH))
		{
			dst = &path_buffer[0];
			return osd_file::error::NONE;
		}
		else
		{
			return osd_file::error::FAILURE;
		}
#else
		std::vector<char> path_buffer(PATH_MAX);
		if (::realpath(path.c_str(), &path_buffer[0]))
		{
			dst = &path_buffer[0];
			return osd_file::error::NONE;
		}
		else if (path[0] == PATHSEPCH)
		{
			dst = path;
			return osd_file::error::NONE;
		}
		else
		{
			while (!::getcwd(&path_buffer[0], path_buffer.size()))
			{
				if (errno != ERANGE)
					return errno_to_file_error(errno);
				else
					path_buffer.resize(path_buffer.size() * 2);
			}
			dst.assign(&path_buffer[0]).push_back(PATHSEPCH);
			dst.append(path);
			return osd_file::error::NONE;
		}
#endif
	}
	catch (...)
	{
		return osd_file::error::OUT_OF_MEMORY;
	}
}


//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path)
{
	if (!path.empty() && is_path_separator(path[0]))
		return true;
#if !defined(WIN32)
	else if (!path.empty() && (path[0] == '.'))
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

const char *osd_get_volume_name(int idx)
{
	if (idx == 0)
		return "/";
	else
		return nullptr;
	}


//============================================================
//  errno_to_file_error
//============================================================

osd_file::error errno_to_file_error(int error)
{
	switch (error)
	{
	case 0:
		return osd_file::error::NONE;

	case ENOENT:
	case ENOTDIR:
		return osd_file::error::NOT_FOUND;

	case EACCES:
	case EROFS:
#ifndef WIN32
	case ETXTBSY:
#endif
	case EEXIST:
	case EPERM:
	case EISDIR:
	case EINVAL:
		return osd_file::error::ACCESS_DENIED;

	case ENFILE:
	case EMFILE:
		return osd_file::error::TOO_MANY_FILES;

	default:
		return osd_file::error::FAILURE;
	}
}
