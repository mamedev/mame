// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlsocket.c - SDL socket (inet, unix domain) access
//  functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "posixfile.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>


namespace {

char const *const posixfile_socket_identifier  = "socket.";
char const *const posixfile_domain_identifier  = "domain.";


class posix_osd_socket : public osd_file
{
public:
	posix_osd_socket(posix_osd_socket const &) = delete;
	posix_osd_socket(posix_osd_socket &&) = delete;
	posix_osd_socket& operator=(posix_osd_socket const &) = delete;
	posix_osd_socket& operator=(posix_osd_socket &&) = delete;

	posix_osd_socket(int sock, bool listening) noexcept
		: m_listener(listening ? sock : -1)
		, m_sock(listening ? -1 : sock)
		, m_eof(false)
	{
		assert(sock >= 0);
	}

	virtual ~posix_osd_socket()
	{
		if (m_sock >= 0)
			::close(m_sock);
		if (m_listener >= 0)
			::close(m_listener);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		// once the client has finished sending, watch for the next one instead
		int const fd = ((m_sock < 0) || m_eof) ? m_listener : m_sock;
		assert(fd >= 0);

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		struct timeval timeout;
		timeout.tv_sec = timeout.tv_usec = 0;

		if (::select(fd + 1, &readfds, nullptr, nullptr, &timeout) < 0)
		{
			return std::error_condition(errno, std::generic_category());
		}
		else if (FD_ISSET(fd, &readfds))
		{
			if (fd == m_sock)
			{
				// connected socket
				ssize_t const result = ::read(m_sock, buffer, count);
				if (result < 0)
				{
					std::error_condition const err(errno, std::generic_category());
					if ((m_listener < 0) || !connection_lost(err))
						return err;

					// report a lost connection as end of file, as for one the peer closed
					disconnect();
					actual = 0;
					return std::error_condition();
				}
				else
				{
					// end of file: the peer has finished sending, but may still be reading
					if (!result && count && (m_listener >= 0))
						m_eof = true;
					actual = std::uint32_t(size_t(result));
					return std::error_condition();
				}
			}
			else
			{
				// listening socket - accept the next connection, replacing a client that has finished sending
				int const accepted = ::accept(m_listener, nullptr, nullptr);
				if (accepted < 0)
				{
					return std::error_condition(errno, std::generic_category());
				}
				else
				{
					disconnect();
					m_sock = accepted;
					actual = 0;

					return std::error_condition();
				}
			}
		}
		else
		{
			// no data available
			actual = 0;
			return std::errc::operation_would_block;
		}
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		if (m_sock >= 0)
		{
			// connected socket
			ssize_t const result = ::write(m_sock, buffer, count);
			if (result < 0)
			{
				// once the peer has finished sending, a failed write ends the connection
				std::error_condition const err(errno, std::generic_category());
				if (m_eof && connection_lost(err))
					disconnect();
				return err;
			}

			actual = std::uint32_t(size_t(result));
			return std::error_condition();
		}
		else
		{
			// not connected
			actual = 0;
			return std::errc::not_connected;
		}
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// doesn't make sense on socket
		return std::errc::bad_file_descriptor;
	}

	virtual std::error_condition flush() noexcept override
	{
		// there's no simple way to flush buffers on a socket anyway
		return std::error_condition();
	}

private:
	static bool connection_lost(std::error_condition const &err) noexcept
	{
		return
				(std::errc::interrupted != err) &&
				(std::errc::operation_would_block != err) &&
				(std::errc::resource_unavailable_try_again != err) &&
				(std::errc::no_buffer_space != err) &&
				(std::errc::not_enough_memory != err);
	}

	void disconnect() noexcept
	{
		// a listening socket goes back to accepting; an outgoing connection stays as it is
		if ((m_listener >= 0) && (m_sock >= 0))
		{
			::close(m_sock);
			m_sock = -1;
			m_eof = false;
		}
	}

	int     m_listener; // -1 for an outgoing connection
	int     m_sock;     // -1 while waiting for a connection
	bool    m_eof;      // client has finished sending
};


template <typename T>
std::error_condition create_socket(T const &sa, int sock, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	osd_file::ptr result;
	if (openflags & OPEN_FLAG_CREATE)
	{
		// listening socket support
		// bind socket...
		if (::bind(sock, reinterpret_cast<struct sockaddr const *>(&sa), sizeof(sa)) < 0)
		{
			std::error_condition binderr(errno, std::generic_category());
			::close(sock);
			return binderr;
		}

		// start to listen...
		if (::listen(sock, 1) < 0)
		{
			std::error_condition lstnerr(errno, std::generic_category());
			::close(sock);
			return lstnerr;
		}

		// mark socket as "listening"
		result.reset(new (std::nothrow) posix_osd_socket(sock, true));
	}
	else
	{
		if (::connect(sock, reinterpret_cast<struct sockaddr const *>(&sa), sizeof(sa)) < 0)
		{
			std::error_condition connerr(errno, std::generic_category());
			::close(sock);
			return connerr;
		}
		result.reset(new (std::nothrow) posix_osd_socket(sock, false));
	}

	if (!result)
	{
		::close(sock);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = 0;
	return std::error_condition();
}

} // anonymous namespace


/*
    Checks whether the path is a socket specification. A valid socket
    specification has the format "socket." host ":" port. Host may be simple
    or fully qualified. Port must be between 1 and 65535.
*/
bool posix_check_socket_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), posixfile_socket_identifier, strlen(posixfile_socket_identifier)) == 0 &&
		strchr(path.c_str(), ':') != nullptr) return true;
	return false;
}


bool posix_check_domain_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), posixfile_domain_identifier, strlen(posixfile_domain_identifier)) == 0)
		return true;
	return false;
}


std::error_condition posix_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	char hostname[256];
	int port;
	std::sscanf(&path[strlen(posixfile_socket_identifier)], "%255[^:]:%d", hostname, &port);

	struct hostent const *const localhost = ::gethostbyname(hostname);
	if (!localhost)
		return std::errc::no_such_file_or_directory;

	struct sockaddr_in sai;
	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *reinterpret_cast<struct in_addr *>(localhost->h_addr);

	int const sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return std::error_condition(errno, std::generic_category());

	int const flag = 1;
	if ((::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0) ||
		(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0))
	{
		std::error_condition sockopterr(errno, std::generic_category());
		::close(sock);
		return sockopterr;
	}

	return create_socket(sai, sock, openflags, file, filesize);
}


std::error_condition posix_open_domain(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	struct sockaddr_un sau;
	memset(&sau, 0, sizeof(sau));
	sau.sun_family = AF_UNIX;
	strncpy(sau.sun_path, &path.c_str()[strlen(posixfile_domain_identifier)], sizeof(sau.sun_path)-1);

	int const sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		return std::error_condition(errno, std::generic_category());

	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
	{
		std::error_condition cntlerr(errno, std::generic_category());
		::close(sock);
		return cntlerr;
	}

	return create_socket(sau, sock, openflags, file, filesize);
}
