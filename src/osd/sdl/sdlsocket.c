//============================================================
//
//  sdlsocket.c - SDL socket (inet) access functions
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef SDLMAME_WIN32
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#endif
#include <errno.h>

#include "emu.h"
#include "sdlfile.h"

const char *sdlfile_socket_identifier  = "/dev/socket";

file_error sdl_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
#ifndef SDLMAME_WIN32
	char hostname[256];
	struct hostent *localhost;
	struct sockaddr_in sai;
	int flag = 1;
	int port;
   
	sscanf( path+strlen(sdlfile_socket_identifier), ":%255[^:]:%d", hostname, &port );
      
	printf("Connecting to server '%s' on port '%d'\n", hostname, port);
   
	if (((*file)->handle = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}

	if (setsockopt((*file)->handle, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}

	localhost = gethostbyname(hostname);

	memset(&sai, 0, sizeof(sai));
	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr = *((struct in_addr *)localhost->h_addr);

	if (connect((*file)->handle, (struct sockaddr *)&sai, sizeof(struct sockaddr)) == -1)
	{
		return FILERR_ACCESS_DENIED;
	}
   
	*filesize = 0;
#endif  
	return FILERR_NONE;
}

file_error sdl_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
#ifndef SDLMAME_WIN32
	ssize_t result;
	char line[80];
	struct timeval timeout;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(file->handle, &readfds);
	timeout.tv_sec = timeout.tv_usec = 0;

	if (select(file->handle + 1, &readfds, NULL, NULL, &timeout) < 0)
	{
		sprintf(line, "%s : %s : %d ", __func__, __FILE__,  __LINE__);
		perror(line);
		return error_to_file_error(errno);
	}
	else if (FD_ISSET(file->handle, &readfds))
	{
		result = read(file->handle, buffer, count);
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
#ifndef SDLMAME_WIN32
	UINT32 result;  

	result = write(file->handle, buffer, count);

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

file_error sdl_close_socket(osd_file *file)
{
#ifndef SDLMAME_WIN32
	close(file->handle);
	osd_free(file);
#endif	
	return FILERR_NONE;
}
