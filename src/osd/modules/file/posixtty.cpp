// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdltty_unix.c - SDL pseudo tty access functions
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
#include <termios.h>

#include <termios.h>



namespace {

char const *const posix_tty_identifier  = "/dev/tty";


class posix_osd_tty : public osd_file
{
public:
	posix_osd_tty(posix_osd_tty const &) = delete;
	posix_osd_tty(posix_osd_tty &&) = delete;
	posix_osd_tty& operator=(posix_osd_tty const &) = delete;
	posix_osd_tty& operator=(posix_osd_tty &&) = delete;

	posix_osd_tty(int fd) : m_fd(fd)
	{
		assert(m_fd >= 0);
	}

	virtual ~posix_osd_tty()
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


bool posix_check_tty_path(std::string const &path) noexcept
{
	return strncmp(path.c_str(), posix_tty_identifier, strlen(posix_tty_identifier)) == 0;
}


std::error_condition  posix_open_tty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
#if defined(__ANDROID__)
	return std::errc::not_supported; // TODO: revisit this error code
#else // defined(__ANDROID__)
	struct termios attr;
	std::memset(&attr, 0, sizeof(attr));
	::cfmakeraw(&attr);
	int fd = -1;
	int access = O_NOCTTY;
	if (openflags & OPEN_FLAG_WRITE)
		access |= (openflags & OPEN_FLAG_READ) ? O_RDWR : O_WRONLY;
	else if (openflags & OPEN_FLAG_READ)
		access |= O_RDONLY;
	else
		return std::errc::invalid_argument;
	fd = open(path.c_str(), access);
	if (fd < 0) 
	{
		return std::error_condition(errno, std::generic_category());
	}
	// No modem signals 
	cfmakeraw(&attr);
	attr.c_cflag |= (CLOCAL | CREAD);
	attr.c_oflag &= ~OPOST;
	attr.c_cc[VMIN] = 0;
	attr.c_cc[VTIME] = 30;
	tcsetattr(fd, TCSAFLUSH, &attr);
	sleep(2); //required to make flush work, for some reason
	tcflush(fd, TCIOFLUSH); 

	try
	{
		file = std::make_unique<posix_osd_tty>(fd);
		filesize = 0;
		return std::error_condition();
	}
	catch (...)
	{
		::close(fd);
		return std::errc::not_enough_memory;
	}
#endif // defined(__ANDROID__)
}
