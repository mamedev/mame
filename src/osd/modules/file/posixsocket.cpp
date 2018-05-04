// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlsocket.c - SDL socket (inet) access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "posixfile.h"

#include <cassert>
#include <cerrno>
#include <cstdio>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>


namespace {
char const *const posixfile_socket_identifier  = "socket.";


class posix_osd_socket : public osd_file
{
public:
	posix_osd_socket(posix_osd_socket const &) = delete;
	posix_osd_socket(posix_osd_socket &&) = delete;
	posix_osd_socket& operator=(posix_osd_socket const &) = delete;
	posix_osd_socket& operator=(posix_osd_socket &&) = delete;

	posix_osd_socket(int sock, bool listening)
		: m_sock(sock)
		, m_listening(listening)
	{
		assert(m_sock >= 0);
	}

	virtual ~posix_osd_socket()
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


/*
    Checks whether the path is a socket specification. A valid socket
    specification has the format "socket." host ":" port. Host may be simple
    or fully qualified. Port must be between 1 and 65535.
*/
bool posix_check_socket_path(std::string const &path)
{
	if (strncmp(path.c_str(), posixfile_socket_identifier, strlen(posixfile_socket_identifier)) == 0 &&
		strchr(path.c_str(), ':') != nullptr) return true;
	return false;
}


osd_file::error posix_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize)
{
	char hostname[256];
	int port;
	std::sscanf(&path[strlen(posixfile_socket_identifier)], "%255[^:]:%d", hostname, &port);

	struct hostent const *const localhost = ::gethostbyname(hostname);
	if (!localhost)
		return osd_file::error::NOT_FOUND;

	struct sockaddr_in sai;
	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *reinterpret_cast<struct in_addr *>(localhost->h_addr);

	int const sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return errno_to_file_error(errno);

	int const flag = 1;
	if (::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
	{
		int const err = errno;
		::close(sock);
		return errno_to_file_error(err);
	}

	if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
	{
		int const err = errno;
		::close(sock);
		return errno_to_file_error(err);
	}


	// listening socket support
	if (openflags & OPEN_FLAG_CREATE)
	{
		//printf("Listening for client at '%s' on port '%d'\n", hostname, port);
		// bind socket...
		if (::bind(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(struct sockaddr)) < 0)
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
			file = std::make_unique<posix_osd_socket>(sock, true);
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
		//printf("Connecting to server '%s' on port '%d'\n", hostname, port);
		if (::connect(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(struct sockaddr)) < 0)
		{
			::close(sock);
			return osd_file::error::ACCESS_DENIED; // have to return this value or bitb won't try to bind on connect failure
		}
		try
		{
			file = std::make_unique<posix_osd_socket>(sock, false);
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
