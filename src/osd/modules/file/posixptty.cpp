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
#include <climits>
#include <unistd.h>
#include <cstdlib>

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <termios.h>
#include <libutil.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <termios.h>
#include <util.h>
#elif defined(__linux__) || defined(__EMSCRIPTEN__) || defined(__GLIBC__)
#include <pty.h>
#elif defined(__HAIKU__)
#include <bsd/pty.h>
#elif defined(__sun)
#include <sys/types.h>
#include <stropts.h>
#include <sys/conf.h>
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

	posix_osd_ptty(int fd) noexcept : m_fd(fd)
	{
		assert(m_fd >= 0);
	}

	virtual ~posix_osd_ptty()
	{
		::close(m_fd);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		ssize_t const result = ::read(m_fd, buffer, count);
		if (result < 0)
			return std::error_condition(errno, std::generic_category());

		actual = std::uint32_t(size_t(result));
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		ssize_t const result = ::write(m_fd, buffer, count);
		if (result < 0)
			return std::error_condition(errno, std::generic_category());

		actual = std::uint32_t(size_t(result));
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// doesn't make sense on ptty
		return std::errc::bad_file_descriptor;
	}

	virtual std::error_condition flush() noexcept override
	{
		// no userspace buffers on read/write
		return std::error_condition();
	}

private:
	int m_fd;
};

} // anonymous namespace


bool posix_check_ptty_path(std::string const &path) noexcept
{
	return strncmp(path.c_str(), posix_ptty_identifier, strlen(posix_ptty_identifier)) == 0;
}


std::error_condition posix_open_ptty(std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize, std::string &name) noexcept
{
#if defined(__ANDROID__)
	return std::errc::not_supported; // TODO: revisit this error code
#else // defined(__ANDROID__)
	// TODO: handling of the slave path is insecure - should use ptsname_r/ttyname_r in a loop
#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
	int access = O_NOCTTY;
	if (openflags & OPEN_FLAG_WRITE)
		access |= (openflags & OPEN_FLAG_READ) ? O_RDWR : O_WRONLY;
	else if (openflags & OPEN_FLAG_READ)
		access |= O_RDONLY;
	else
		return std::errc::invalid_argument;

	int const masterfd = ::posix_openpt(access);
	if (masterfd < 0)
		return std::error_condition(errno, std::generic_category());

	// grant access to slave device and check that it can be opened
	char const *slavepath;
	int slavefd;
	if ((::grantpt(masterfd) < 0) ||
		(::unlockpt(masterfd) < 0) ||
		((slavepath = ::ptsname(masterfd)) == nullptr) ||
		((slavefd = ::open(slavepath, O_RDWR | O_NOCTTY)) < 0))
	{
		std::error_condition err(errno, std::generic_category());
		::close(masterfd);
		return err;
	}

	// check that it's possible to stack BSD-compatibility STREAMS modules
	if ((::ioctl(slavefd, I_PUSH, "ptem") < 0) ||
		(::ioctl(slavefd, I_PUSH, "ldterm") < 0) ||
		(::ioctl(slavefd, I_PUSH, "ttcompat") < 0))
	{
		std::error_condition err(errno, std::generic_category());
		::close(slavefd);
		::close(masterfd);
		return err;
	}
#else
	struct termios tios;
	std::memset(&tios, 0, sizeof(tios));
	::cfmakeraw(&tios); // TODO: this is a non-standard BSDism - should set flags some other way

	int masterfd = -1, slavefd = -1;
	char slavepath[PATH_MAX];
	if (::openpty(&masterfd, &slavefd, slavepath, &tios, nullptr) < 0)
		return std::error_condition(errno, std::generic_category());
#endif

	::close(slavefd);

	int const oldflags = ::fcntl(masterfd, F_GETFL, 0);
	if (oldflags < 0)
	{
		std::error_condition err(errno, std::generic_category());
		::close(masterfd);
		return err;
	}

	if (::fcntl(masterfd, F_SETFL, oldflags | O_NONBLOCK) < 0)
	{
		std::error_condition err(errno, std::generic_category());
		::close(masterfd);
		return err;
	}

	try
	{
		name = slavepath;
		file = std::make_unique<posix_osd_ptty>(masterfd);
		filesize = 0;
		return std::error_condition();
	}
	catch (...)
	{
		::close(masterfd);
		return std::errc::not_enough_memory;
	}
#endif // defined(__ANDROID__)
}
