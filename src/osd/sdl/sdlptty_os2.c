// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlptty_os2 - SDL psuedo tty access functions
//  (OS/2 has no pttys - maybe named pipes?)
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

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

file_error sdl_slave_name_ptty(osd_file *file)
{
		return FILERR_ACCESS_DENIED;
}
