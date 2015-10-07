// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlfile.h - SDL file access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdcore.h"

//============================================================
//  ENUM DEFINITIONS
//============================================================
enum
{
	SDLFILE_FILE = 0,
	SDLFILE_SOCKET,
	SDLFILE_PTTY
};

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_file
{
	int handle;
	int socket;
	int type;
	char    filename[1];
};

//============================================================
//  PROTOTYPES
//============================================================

bool sdl_check_socket_path(const char *path);
file_error sdl_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error sdl_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_write_socket(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_close_socket(osd_file *file);

file_error sdl_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error sdl_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_close_ptty(osd_file *file);
file_error sdl_slave_name_ptty(osd_file *file , char *name , size_t name_len);

file_error error_to_file_error(UINT32 error);
