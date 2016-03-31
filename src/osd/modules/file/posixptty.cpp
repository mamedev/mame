// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlptty_unix.c - SDL pseudo tty access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "posixfile.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <termios.h>
#include <libutil.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__) || defined(__ANDROID__)
#include <termios.h>
#include <util.h>
#elif defined(__linux__) || defined(EMSCRIPTEN)
#include <pty.h>
#elif defined(__HAIKU__)
#include <bsd/pty.h>
#endif


namespace {
#if defined(__APPLE__)
char const *const posix_ptty_identifier  = "/dev/pty";
#else
char const *const posix_ptty_identifier  = "/dev/pts";
#endif


class posix_osd_ptty : public osd_file
{
public:
	posix_osd_ptty(posix_osd_ptty const &) = delete;
	posix_osd_ptty(posix_osd_ptty &&) = delete;
	posix_osd_ptty& operator=(posix_osd_ptty const &) = delete;
	posix_osd_ptty& operator=(posix_osd_ptty &&) = delete;

	posix_osd_ptty(int fd) : m_fd(fd)
	{
		assert(m_fd >= 0);
	}

	virtual ~posix_osd_ptty()
	{
		::close(m_fd);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		ssize_t const result = ::read(m_fd, buffer, count);
		if (result < 0)
			return errno_to_file_error(errno);

		actual = std::uint32_t(size_t(result));
		return error::NONE;
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		ssize_t const result = ::write(m_fd, buffer, count);
		if (result < 0)
			return errno_to_file_error(errno);

		actual = std::uint32_t(size_t(result));
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		// doesn't make sense on ptty
		return error::INVALID_ACCESS;
	}

	virtual error flush() override
	{
		// no userspace buffers on read/write
		return error::NONE;
	}

private:
	int m_fd;
};

} // anonymous namespace


bool posix_check_ptty_path(std::string const &path)
{
	return strncmp(path.c_str(), posix_ptty_identifier, strlen(posix_ptty_identifier)) == 0;
}


osd_file::error posix_open_ptty(std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize, std::string &name)
{
#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
	int access = O_NOCTTY;
	if (openflags & OPEN_FLAG_WRITE)
		access |= (openflags & OPEN_FLAG_READ) ? O_RDWR : O_WRONLY;
	else if (openflags & OPEN_FLAG_READ)
		access |= O_RDONLY;
	else
		return error::INVALID_ACCESS;

	int const masterfd = ::posix_openpt(access);
	if (masterfd < 0)
		return errno_to_file_error(errno);

	// grant access to slave device and check that it can be opened
	char const *slavepath;
	int slavefd;
	if ((::grantpt(masterfd) < 0) ||
		(::unlockpt(masterfd) < 0) ||
		((slavepath = ::ptsname(masterfd)) == nullptr) ||
		((slavefd = ::open(slavepath, O_RDWR | O_NOCTTY)) < 0))
	{
		int const err = errno;
		::close(masterfd);
		return errno_to_file_error(err);
	}

	// check that it's possible to stack BSD-compatibility STREAMS modules
	if ((::ioctl(slavefd, I_PUSH, "ptem") < 0) ||
		(::ioctl(slavefd, I_PUSH, "ldterm") < 0) ||
		(::ioctl(slavefd, I_PUSH, "ttcompat") < 0))
	{
		int const err = errno;
		::close(slavefd);
		::close(masterfd);
		return errno_to_file_error(err);
	}
#elif defined(__ANDROID__)
	int masterfd = -1, slavefd = -1;
	char slavepath[PATH_MAX];
#else
	struct termios tios;
	std::memset(&tios, 0, sizeof(tios));
	::cfmakeraw(&tios);

	int masterfd = -1, slavefd = -1;
	char slavepath[PATH_MAX];
	if (::openpty(&masterfd, &slavefd, slavepath, &tios, nullptr) < 0)
		return errno_to_file_error(errno);
#endif

	::close(slavefd);

	int const oldflags = ::fcntl(masterfd, F_GETFL, 0);
	if (oldflags < 0)
	{
		int const err = errno;
		::close(masterfd);
		return errno_to_file_error(err);
	}

	if (::fcntl(masterfd, F_SETFL, oldflags | O_NONBLOCK) < 0)
	{
		int const err = errno;
		::close(masterfd);
		return errno_to_file_error(err);
	}

	try
	{
		name = slavepath;
		file = std::make_unique<posix_osd_ptty>(masterfd);
		filesize = 0;
		return osd_file::error::NONE;
	}
	catch (...)
	{
		::close(masterfd);
		return osd_file::error::OUT_OF_MEMORY;
	}
}
