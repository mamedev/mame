// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winsocket.c - Windows socket (inet) access functions
//
//============================================================

#include "winfile.h"

// MAMEOS headers
#include "winutil.h"

// MAME headers
#include "osdcore.h"

#include <cassert>
#include <cstdio>
#include <cstring>

// standard windows headers
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <cstdlib>
#include <cctype>

#define DEBUG_SOCKET (0)

namespace {

char const *const winfile_socket_identifier  = "socket.";
char const *const winfile_udp_identifier = "udp.";


class win_osd_socket : public osd_file
{
public:
	win_osd_socket(win_osd_socket const &) = delete;
	win_osd_socket(win_osd_socket &&) = delete;
	win_osd_socket& operator=(win_osd_socket const &) = delete;
	win_osd_socket& operator=(win_osd_socket &&) = delete;

	win_osd_socket(SOCKET s, bool l, uint32_t dest = 0, uint32_t dport = 0) noexcept
		: m_socket(s)
		, m_listening(l)
		, m_dest(dest)
		, m_dport(dport)
	{
		assert(INVALID_SOCKET != m_socket);
	}

	virtual ~win_osd_socket() override
	{
		closesocket(m_socket);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(m_socket, &readfds);

		struct timeval timeout;
		timeout.tv_sec = timeout.tv_usec = 0;

		if (select(m_socket + 1, &readfds, nullptr, nullptr, &timeout) < 0)
		{
			return wsa_error_to_file_error(WSAGetLastError());
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
					return std::error_condition();
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

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		if (!m_dest) {
			auto const result = send(m_socket, reinterpret_cast<const char*>(buffer), length, 0);
			if (result < 0)
				return wsa_error_to_file_error(WSAGetLastError());
			actual = result;
		}
		else
		{
			sockaddr_in dest;
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = m_dest;
			dest.sin_port = htons(m_dport);
			//printf("Sending to %d bytes %s port %d\n", length, inet_ntoa(dest.sin_addr), m_dport);
			auto const result = sendto(m_socket, reinterpret_cast<const char*>(buffer), length, 0, reinterpret_cast <sockaddr*> (&dest), sizeof(dest));
			if (result < 0)
				return wsa_error_to_file_error(WSAGetLastError());
			actual = result;
		}
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// doesn't make sense for a socket
		return std::errc::bad_file_descriptor;
	}

	virtual std::error_condition flush() noexcept override
	{
		// no buffers to flush
		return std::error_condition();
	}

	static std::error_condition wsa_error_to_file_error(int err)
	{
		// TODO: determine if there's a better way to do this
		switch (err)
		{
		case 0:                     return std::error_condition();
		case WSA_NOT_ENOUGH_MEMORY: return std::errc::not_enough_memory;
		case WSA_INVALID_PARAMETER: return std::errc::invalid_argument;
		case WSAEINTR:              return std::errc::interrupted;
		case WSAEBADF:              return std::errc::bad_file_descriptor;
		case WSAEACCES:             return std::errc::permission_denied;
		case WSAEFAULT:             return std::errc::bad_address;
		case WSAEINVAL:             return std::errc::invalid_argument;
		case WSAEMFILE:             return std::errc::too_many_files_open;
		case WSAENETRESET:          return std::errc::network_reset;
		case WSAECONNABORTED:       return std::errc::connection_aborted;
		case WSAEWOULDBLOCK:        return std::errc::operation_would_block;
		case WSAEINPROGRESS:        return std::errc::operation_in_progress;
		case WSAEALREADY:           return std::errc::connection_already_in_progress;
		case WSAENOTSOCK:           return std::errc::not_a_socket;
		case WSAEDESTADDRREQ:       return std::errc::destination_address_required;
		case WSAEMSGSIZE:           return std::errc::message_size;
		case WSAEPROTOTYPE:         return std::errc::wrong_protocol_type;
		case WSAENOPROTOOPT:        return std::errc::no_protocol_option;
		case WSAEPROTONOSUPPORT:    return std::errc::protocol_not_supported;
		case WSAEOPNOTSUPP:         return std::errc::operation_not_supported;
		case WSAEAFNOSUPPORT:       return std::errc::address_family_not_supported;
		case WSAEADDRINUSE:         return std::errc::address_in_use;
		case WSAEADDRNOTAVAIL:      return std::errc::address_not_available;
		case WSAENETDOWN:           return std::errc::network_down;
		case WSAENETUNREACH:        return std::errc::network_unreachable;
		case WSAECONNRESET:         return std::errc::connection_reset;
		case WSAENOBUFS:            return std::errc::no_buffer_space;
		case WSAEISCONN:            return std::errc::already_connected;
		case WSAENOTCONN:           return std::errc::not_connected;
		case WSAETIMEDOUT:          return std::errc::timed_out;
		case WSAECONNREFUSED:       return std::errc::connection_refused;
		case WSAEHOSTUNREACH:       return std::errc::host_unreachable;
		case WSAECANCELLED:         return std::errc::operation_canceled;

		// TODO: better default error code?
		default:                    return std::errc::io_error;
		}
	}

private:
	SOCKET  m_socket;
	bool    m_listening;
	uint32_t m_dest;
	uint32_t m_dport;
};

} // anonymous namespace


bool win_init_sockets() noexcept
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


void win_cleanup_sockets() noexcept
{
	WSACleanup();
}


bool win_check_socket_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), winfile_socket_identifier, strlen(winfile_socket_identifier)) == 0 &&
		strchr(path.c_str(), ':') != nullptr) return true;
	return false;
}


std::error_condition win_open_tcp_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	if (DEBUG_SOCKET)
		printf("Trying to open tcp socket %s\n", path.c_str());

	char hostname[256];
	int port;
	std::sscanf(&path[strlen(winfile_socket_identifier)], "%255[^:]:%d", hostname, &port);

	struct hostent const *const localhost = gethostbyname(hostname);
	if (!localhost)
		return std::errc::no_such_file_or_directory;

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
	osd_file::ptr result;
	if (openflags & OPEN_FLAG_CREATE)
	{
		if (DEBUG_SOCKET)
			printf("Listening for client at '%s' on port '%d'\n", hostname, port);
		// bind socket...
		if (bind(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(sai)) == SOCKET_ERROR)
		{
			if (DEBUG_SOCKET)
				printf("bind error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}

		// start to listen...
		if (listen(sock, 1) == SOCKET_ERROR)
		{
			if (DEBUG_SOCKET)
				printf("listen error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}

		// mark socket as "listening"
		result.reset(new (std::nothrow) win_osd_socket(sock, true));
	}
	else
	{
		if (DEBUG_SOCKET)
			printf("Connecting to server '%s' on port '%d'\n", hostname, port);
		if (connect(sock, reinterpret_cast<struct sockaddr const *>(&sai), sizeof(sai)) == SOCKET_ERROR)
		{
			if (DEBUG_SOCKET)
				printf("connect error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}
		result.reset(new (std::nothrow) win_osd_socket(sock, false));
	}

	if (!result)
	{
		closesocket(sock);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = 0;
	return std::error_condition();
}

bool win_check_udp_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), winfile_udp_identifier, strlen(winfile_udp_identifier)) == 0 &&
		strchr(path.c_str(), ':') != nullptr) return true;
	return false;
}


std::error_condition win_open_udp_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	if (DEBUG_SOCKET)
		printf("Trying to open udp socket %s\n", path.c_str());
	char hostname[256];
	int port;
	std::sscanf(&path[strlen(winfile_udp_identifier)], "%255[^:]:%d", hostname, &port);
	//std::sscanf(path.c_str(), "%255[^:]:%d", hostname, &port);

	struct hostent const* const localhost = gethostbyname(hostname);
	if (!localhost)
		return std::errc::no_such_file_or_directory;
	struct sockaddr_in sai;
	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *reinterpret_cast<struct in_addr*>(localhost->h_addr);
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock)
		return win_osd_socket::wsa_error_to_file_error(WSAGetLastError());

	int const flag = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag), sizeof(flag)) == SOCKET_ERROR)
	{
		if (DEBUG_SOCKET)
			printf("SO_REUSEADDR error\n");
		int const err = WSAGetLastError();
		closesocket(sock);
		return win_osd_socket::wsa_error_to_file_error(err);
	}

	// local socket support
	osd_file::ptr result;
	if (openflags & OPEN_FLAG_CREATE)
	{
		sai.sin_addr.s_addr = htonl(INADDR_ANY);
		if (DEBUG_SOCKET)
			printf("Listening on '%s' on port '%d'\n", hostname, port);
		// bind socket...
		if (bind(sock, reinterpret_cast<struct sockaddr const*>(&sai), sizeof(struct sockaddr)) == SOCKET_ERROR)
		{
			if (DEBUG_SOCKET)
				printf("bind error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}
		// Setup multicast
		struct ip_mreq mreq;
		mreq.imr_multiaddr = *reinterpret_cast<struct in_addr*>(localhost->h_addr);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (mreq.imr_multiaddr.S_un.S_un_b.s_b1 >= 224 && mreq.imr_multiaddr.S_un.S_un_b.s_b1 <= 239)
		{
			if (DEBUG_SOCKET)
				printf("Setting up udp multicast %s\n", hostname);
			if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
				if (DEBUG_SOCKET)
					printf("IP_ADD_MEMBERSHIP error\n");
				int const err = WSAGetLastError();
				closesocket(sock);
				return win_osd_socket::wsa_error_to_file_error(err);
			}
		}
		// Disable loopback on multicast
		char const loop = 0;
		if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)))
		{
			if (DEBUG_SOCKET)
				printf("IP_MULTICAST_LOOP error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}
		// Set ttl on multicast
		char const ttl = 15;
		if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
		{
			if (DEBUG_SOCKET)
				printf("IP_MULTICAST_TTL error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}
		result.reset(new (std::nothrow) win_osd_socket(sock, false, (*reinterpret_cast<struct in_addr*>(localhost->h_addr)).s_addr, port));
		if (!result)
		{
			if (DEBUG_SOCKET)
				printf("connect error\n");
			int const err = WSAGetLastError();
			closesocket(sock);
			return win_osd_socket::wsa_error_to_file_error(err);
		}
	}
	if (!result)
	{
		closesocket(sock);
		return std::errc::permission_denied;
	}
	file = std::move(result);
	filesize = 0;
	return std::error_condition();
}
