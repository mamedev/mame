// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winsocket.c - Windows socket (inet) access functions
//
//============================================================

#define WIN32_LEAN_AND_MEAN

#include "winfile.h"

// MAMEOS headers
#include "winutil.h"

// MAME headers
#include "osdcore.h"

#include <cassert>
#include <cstdio>

// standard windows headers
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdlib.h>
#include <ctype.h>


namespace {
char const *const winfile_socket_identifier  = "socket.";


class win_osd_socket : public osd_file
{
public:
	win_osd_socket(win_osd_socket const &) = delete;
	win_osd_socket(win_osd_socket &&) = delete;
	win_osd_socket& operator=(win_osd_socket const &) = delete;
	win_osd_socket& operator=(win_osd_socket &&) = delete;

	win_osd_socket(SOCKET s, bool l)
		: m_socket(s)
		, m_listening(l)
	{
		assert(INVALID_SOCKET != m_socket);
	}

	virtual ~win_osd_socket() override
	{
		closesocket(m_socket);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(m_socket, &readfds);

		struct timeval timeout;
		timeout.tv_sec = timeout.tv_usec = 0;

		if (select(m_socket + 1, &readfds, nullptr, nullptr, &timeout) < 0)
		{
			char line[80];
			std::sprintf(line, "win_read_socket : %s : %d ", __FILE__,  __LINE__);
			std::perror(line);
			return error::FAILURE;
		}
		else if (FD_ISSET(m_socket, &readfds))
		{
			if (!m_listening)
			{
				// connected socket
				int const result = recv(m_socket, (char*)buffer, length, 0);
				if (result < 0)
				{
					return wsa_error_to_file_error(WSAGetLastError());
				}
				else
				{
					actual = result;
					return error::NONE;
				}
			}
			else
			{
				// listening socket
				SOCKET const accepted = accept(m_socket, nullptr, nullptr);
				if (INVALID_SOCKET == accepted)
				{
					return wsa_error_to_file_error(WSAGetLastError());
				}
				else
				{
					closesocket(m_socket);
					m_socket = accepted;
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

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		auto const result = send(m_socket, reinterpret_cast<const char *>(buffer), length, 0);
		if (result < 0)
			return wsa_error_to_file_error(WSAGetLastError());

		actual = result;
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		// doesn't make sense for a socket
		return error::INVALID_ACCESS;
	}

	virtual error flush() override
	{
		// no buffers to flush
		return error::NONE;
	}

	static error wsa_error_to_file_error(int err)
	{
		switch (err)
		{
		case 0:                 return error::NONE;
		case WSAEACCES:         return error::ACCESS_DENIED;
		case WSAEADDRINUSE:     return error::ALREADY_OPEN;
		case WSAEADDRNOTAVAIL:  return error::NOT_FOUND;
		case WSAECONNREFUSED:   return error::NOT_FOUND;
		case WSAEHOSTUNREACH:   return error::NOT_FOUND;
		case WSAENETUNREACH:    return error::NOT_FOUND;
		default:                return error::FAILURE;
		}
	}

private:
	SOCKET  m_socket;
	bool    m_listening;
};

} // anonymous namespace


bool win_init_sockets()
{
	WSADATA wsaData;
	WORD const version = MAKEWORD(2, 0);
	int const error = WSAStartup(version, &wsaData);

	// check for error
	if (error)
	{
		// error occurred
		return false;
	}

	// check for correct version
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion ) != 0)
	{
		// incorrect WinSock version
		WSACleanup();
		return false;
	}

	// WinSock has been initialized
	return true;
}


void win_cleanup_sockets()
{
	WSACleanup();
}


bool win_check_socket_path(std::string const &path)
{
	if (strncmp(path.c_str(), winfile_socket_identifier, strlen(winfile_socket_identifier)) == 0 &&
		strchr(path.c_str(), ':') != nullptr) return true;
	return false;
}


osd_file::error win_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize)
{
	char hostname[256];
	int port;
	std::sscanf(&path[strlen(winfile_socket_identifier)], "%255[^:]:%d", hostname, &port);

	struct hostent const *const localhost = gethostbyname(hostname);
	if (!localhost)
		return osd_file::error::NOT_FOUND;

	struct sockaddr_in sai;
	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *reinterpret_cast<struct in_addr *>(localhost->h_addr);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock)
		return win_osd_socket::wsa_error_to_file_error(WSAGetLastError());

	int const flag = 1;
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&flag), sizeof(flag)) == SOCKET_ERROR)
	{
		int const err = WSAGetLastError();
		closesocket(sock);
		return win_osd_socket::wsa_error_to_file_error(err);
	}

	// listening socket support
	if (openflags & OPEN_FLAG_CREATE)
	{
		//printf("Listening for client at '%s' on port '%d'\n", hostname, port);
		// bind socket...
		if (bind(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(struct sockaddr)) == SOCKET_ERROR)
		{
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}

		// start to listen...
		if (listen(sock, 1) == SOCKET_ERROR)
		{
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}

		// mark socket as "listening"
		try
		{
			file = std::make_unique<win_osd_socket>(sock, true);
			filesize = 0;
			return osd_file::error::NONE;
		}
		catch (...)
		{
			closesocket(sock);
			return osd_file::error::OUT_OF_MEMORY;
		}
	}
	else
	{
		//printf("Connecting to server '%s' on port '%d'\n", hostname, port);
		if (connect(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(struct sockaddr)) == SOCKET_ERROR)
		{
			closesocket(sock);
			return osd_file::error::ACCESS_DENIED; // have to return this value or bitb won't try to bind on connect failure
		}
		try
		{
			file = std::make_unique<win_osd_socket>(sock, false);
			filesize = 0;
			return osd_file::error::NONE;
		}
		catch (...)
		{
			closesocket(sock);
			return osd_file::error::OUT_OF_MEMORY;
		}
	}
}
