// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlptty_unix.c - SDL psuedo tty access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if (!defined(SDLMAME_SOLARIS) && !(defined(SDLMAME_OS2)))

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#if defined(SDLMAME_FREEBSD) || defined(SDLMAME_DRAGONFLY)
# include <termios.h>
# include <libutil.h>
#elif defined(SDLMAME_NETBSD) || defined(SDLMAME_MACOSX)
# include <util.h>
#elif defined(SDLMAME_OPENBSD)
# include <termios.h>
# include <util.h>
#elif defined(SDLMAME_LINUX) || defined(SDLMAME_EMSCRIPTEN)
# include <pty.h>
#elif defined(SDLMAME_HAIKU)
# include <bsd/pty.h>
#endif

#include "sdlfile.h"

#if defined(SDLMAME_MACOSX)
const char *sdlfile_ptty_identifier  = "/dev/pty";
#else
const char *sdlfile_ptty_identifier  = "/dev/pts";
#endif

file_error sdl_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	int master;
	int aslave;
	char name[100];

	if (openpty(&master, &aslave, name, NULL, NULL) >= 0)
	{
		printf("Slave of device %s is %s\n", path, name );
		fcntl(master, F_SETFL, O_NONBLOCK);
		(*file)->handle = master;
		*filesize = 0;
	}
	else
	{
		return FILERR_ACCESS_DENIED;
	}

	return FILERR_NONE;
}

file_error sdl_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	ssize_t result;

	result = read(file->handle, buffer, count);

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

file_error sdl_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	ssize_t result;
	result = write(file->handle, buffer, count);

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

file_error sdl_close_ptty(osd_file *file)
{
	close(file->handle);
	osd_free(file);

	return FILERR_NONE;
}

#else
#include "sdlfile.h"

const char *sdlfile_ptty_identifier  = "";

file_error sdl_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	return FILERR_ACCESS_DENIED;
}

file_error sdl_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	return FILERR_ACCESS_DENIED;
}

file_error sdl_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	return FILERR_ACCESS_DENIED;
}

file_error sdl_close_ptty(osd_file *file)
{
	return FILERR_ACCESS_DENIED;
}
#endif
