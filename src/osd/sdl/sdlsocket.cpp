// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlsocket.c - SDL socket (inet) access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#include "../windows/winsocket.cpp"
#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>

#include "emu.h"
#include "sdlfile.h"

const char *sdlfile_socket_identifier  = "socket.";

/*
    Checks whether the path is a socket specification. A valid socket
    specification has the format "socket." host ":" port. Host may be simple
    or fully qualified. Port must be between 1 and 65535.
*/
bool sdl_check_socket_path(const char *path)
{
	if (strlen(sdlfile_socket_identifier) > 0 &&
		strncmp(path, sdlfile_socket_identifier, strlen(sdlfile_socket_identifier)) == 0 &&
		strchr(path, ':') != NULL) return true;
	return false;
}

file_error sdl_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	char hostname[256];
	struct hostent *localhost;
	struct sockaddr_in sai;
	int flag = 1;
	int port;

	sscanf( path+strlen(sdlfile_socket_identifier), "%255[^:]:%d", hostname, &port );

//  printf("Connecting to server '%s' on port '%d'\n", hostname, port);

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
	(*file)->handle = -1;
	return FILERR_NONE;
}

file_error sdl_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
#if (!defined(SDLMAME_EMSCRIPTEN))
	ssize_t result;
	char line[80];
	struct timeval timeout;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(file->socket, &readfds);
	timeout.tv_sec = timeout.tv_usec = 0;

	if (select(file->socket + 1, &readfds, NULL, NULL, &timeout) < 0)
	{
		sprintf(line, "%s : %s : %d ", __func__, __FILE__,  __LINE__);
		perror(line);
		return error_to_file_error(errno);
	}
	else if (FD_ISSET(file->socket, &readfds))
	{
		if (file->handle == -1)
		{
			// connected socket
			result = read(file->socket, buffer, count);
		}
		else
		{
			// listening socket
			int AcceptSocket;
			AcceptSocket = accept(file->socket, NULL, NULL);
			if (AcceptSocket < 0)
			{
				return FILERR_FAILURE;
			}
			close(file->socket);
			file->socket = AcceptSocket;
			file->handle = -1;
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
		return error_to_file_error(errno);
	}

	if (actual != NULL )
	{
		*actual = result;
	}
#endif
	return FILERR_NONE;
}

file_error sdl_write_socket(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	ssize_t result;

	result = write(file->socket, buffer, count);

	if (result < 0)
	{
		return error_to_file_error(errno);
	}

	if (actual != NULL )
	{
		*actual = result;
	}
	return FILERR_NONE;
}

file_error sdl_close_socket(osd_file *file)
{
	close(file->socket);
	osd_free(file);
	return FILERR_NONE;
}
#endif
