// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdldomain.c - SDL socket (unix) access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "posixfile.h"

#include <cassert>
#include <cerrno>
#include <cstdio>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>


namespace {
char const *const posixfile_domain_identifier  = "domain.";


class posix_osd_domain : public osd_file
{
public:
	posix_osd_domain(posix_osd_domain const &) = delete;
	posix_osd_domain(posix_osd_domain &&) = delete;
	posix_osd_domain& operator=(posix_osd_domain const &) = delete;
	posix_osd_domain& operator=(posix_osd_domain &&) = delete;

	posix_osd_domain(int sock, bool listening)
		: m_sock(sock)
		, m_listening(listening)
	{
		assert(m_sock >= 0);
	}

	virtual ~posix_osd_domain()
	{
		::close(m_sock);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(m_sock, &readfds);

		struct timeval timeout;
		timeout.tv_sec = timeout.tv_usec = 0;

		if (select(m_sock + 1, &readfds, nullptr, nullptr, &timeout) < 0)
		{
			char line[80];
			std::sprintf(line, "%s : %s : %d ", __func__, __FILE__,  __LINE__);
			std::perror(line);
			return errno_to_file_error(errno);
		}
		else if (FD_ISSET(m_sock, &readfds))
		{
			if (!m_listening)
			{
				// connected socket
				ssize_t const result = ::read(m_sock, buffer, count);
				if (result < 0)
				{
					return errno_to_file_error(errno);
				}
				else
				{
					actual = std::uint32_t(size_t(result));
					return error::NONE;
				}
			}
			else
			{
				// listening socket
				int const accepted = ::accept(m_sock, nullptr, nullptr);
				if (accepted < 0)
				{
					return errno_to_file_error(errno);
				}
				else
				{
					::close(m_sock);
					m_sock = accepted;
					m_listening = false;
					actual = 0;

					return error::NONE;
				}
			}
		}
		else
		{
			return error::FAILURE;
		}
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		ssize_t const result = ::write(m_sock, buffer, count);
		if (result < 0)
			return errno_to_file_error(errno);

		actual = std::uint32_t(size_t(result));
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		// doesn't make sense on socket
		return error::INVALID_ACCESS;
	}

	virtual error flush() override
	{
		// there's no simple way to flush buffers on a socket anyway
		return error::NONE;
	}

private:
	int     m_sock;
	bool    m_listening;
};

} // anonymous namespace


bool posix_check_domain_path(std::string const &path)
{
	if (strncmp(path.c_str(), posixfile_domain_identifier, strlen(posixfile_domain_identifier)) == 0)
		return true;
	return false;
}


osd_file::error posix_open_domain(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize)
{
	struct sockaddr_un sau;
	memset(&sau, 0, sizeof(sau));
	sau.sun_family = AF_UNIX;
	strncpy(sau.sun_path, &path.c_str()[strlen(posixfile_domain_identifier)], sizeof(sau.sun_path)-1);

	int const sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		return errno_to_file_error(errno);

	fcntl(sock, F_SETFL, O_NONBLOCK);

	// listening socket support
	if (openflags & OPEN_FLAG_CREATE)
	{
		if (::bind(sock, reinterpret_cast<struct sockaddr const *>(&sau), sizeof(struct sockaddr_un)) < 0)
		{
			int const err = errno;
			::close(sock);
			return errno_to_file_error(err);
		}

		// start to listen...
		if (::listen(sock, 1) < 0)
		{
			int const err = errno;
			::close(sock);
			return errno_to_file_error(err);
		}

		// mark socket as "listening"
		try
		{
			file = std::make_unique<posix_osd_domain>(sock, true);
			filesize = 0;
			return osd_file::error::NONE;
		}
		catch (...)
		{
			::close(sock);
			return osd_file::error::OUT_OF_MEMORY;
		}
	}
	else
	{
		if (::connect(sock, reinterpret_cast<struct sockaddr const *>(&sau), sizeof(struct sockaddr_un)) < 0)
		{
			::close(sock);
			return osd_file::error::ACCESS_DENIED; // have to return this value or bitb won't try to bind on connect failure
		}
		try
		{
			file = std::make_unique<posix_osd_domain>(sock, false);
			filesize = 0;
			return osd_file::error::NONE;
		}
		catch (...)
		{
			::close(sock);
			return osd_file::error::OUT_OF_MEMORY;
		}
	}
}
