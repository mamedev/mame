// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winsocket.c - Windows socket (inet) access functions
//
//============================================================
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdlib.h>
#include <ctype.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"
#include "winutf8.h"

#include "winfile.h"

const char *winfile_socket_identifier  = "socket.";

bool win_init_sockets()
{
	WSADATA wsaData;
	WORD version;
	int error;

	version = MAKEWORD( 2, 0 );

	error = WSAStartup( version, &wsaData );

	/* check for error */
	if ( error != 0 )
	{
		/* error occurred */
		return FALSE;
	}


	/* check for correct version */
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
			HIBYTE( wsaData.wVersion ) != 0 )
	{
		/* incorrect WinSock version */
		WSACleanup();
		return FALSE;
	}
	/* WinSock has been initialized */
	return TRUE;
}

void win_cleanup_sockets()
{
	WSACleanup();
}

bool win_check_socket_path(const char *path)
{
	if (strlen(winfile_socket_identifier) > 0 &&
		strncmp(path, winfile_socket_identifier, strlen(winfile_socket_identifier)) == 0 &&
		strchr(path, ':') != NULL) return true;
	return false;
}

file_error win_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	char hostname[256];
	struct hostent *localhost;
	struct sockaddr_in sai;
	int flag = 1;
	int port;

	sscanf( path+strlen(winfile_socket_identifier), "%255[^:]:%d", hostname, &port );

	if (((*file)->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}

	if (setsockopt((*file)->socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}
	localhost = gethostbyname(hostname);

	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *((struct in_addr *)localhost->h_addr);

	// listening socket support
	if (openflags & OPEN_FLAG_CREATE)
	{
//      printf("Listening for client at '%s' on port '%d'\n", hostname, port);
		// bind socket...
		if (bind((*file)->socket, (struct sockaddr *)&sai, sizeof(struct sockaddr)) == -1)
		{
			return FILERR_ACCESS_DENIED;
		}

		// start to listen...
		if (listen((*file)->socket, 1) == -1) {
			return FILERR_ACCESS_DENIED;
		}

		// mark socket as "listening"
		(*file)->handle = 0;
		*filesize = 0;
		return FILERR_NONE;
	}

//  printf("Connecting to server '%s' on port '%d'\n", hostname, port);
	if (connect((*file)->socket, (struct sockaddr *)&sai, sizeof(struct sockaddr)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}
	*filesize = 0;
	(*file)->handle = INVALID_HANDLE_VALUE;
	return FILERR_NONE;
}

file_error win_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	int result;
	char line[80];
	struct timeval timeout;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(file->socket, &readfds);
	timeout.tv_sec = timeout.tv_usec = 0;

	if (select(file->socket + 1, &readfds, NULL, NULL, &timeout) < 0)
	{
		sprintf(line, "win_read_socket : %s : %d ", __FILE__,  __LINE__);
		perror(line);
		return win_error_to_mame_file_error(GetLastError());
	}
	else if (FD_ISSET(file->socket, &readfds))
	{
		if (file->handle == INVALID_HANDLE_VALUE)
		{
			// connected socket
			result = recv(file->socket, (char*)buffer, count, 0);
		}
		else
		{
			// listening socket
			SOCKET AcceptSocket;
			AcceptSocket = accept(file->socket, NULL, NULL);
			if (AcceptSocket == INVALID_SOCKET)
			{
				return FILERR_FAILURE;
			}
			closesocket(file->socket);
			file->socket = AcceptSocket;
			file->handle = INVALID_HANDLE_VALUE;
			if (actual != NULL )
			{
				*actual = 0;
			}

			return FILERR_NONE;
		}
	}
	else
	{
		return FILERR_FAILURE;
	}

	if (result < 0)
	{
		return win_error_to_mame_file_error(GetLastError());
	}

	if (actual != NULL )
	{
		*actual = result;
	}
	return FILERR_NONE;
}

file_error win_write_socket(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	int result;
	result = send(file->socket, (const char*)buffer, count, 0);
	if (result < 0)
	{
		return win_error_to_mame_file_error(GetLastError());
	}

	if (actual != NULL )
	{
		*actual = result;
	}
	return FILERR_NONE;
}

file_error win_close_socket(osd_file *file)
{
	closesocket(file->socket);
	return FILERR_NONE;
}
