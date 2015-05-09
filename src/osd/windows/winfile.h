// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winfile.h - File access functions
//
//============================================================
#ifndef __WINFILE__
#define __WINFILE__

#include <winsock2.h>
#include "osdcore.h"

//============================================================
//  ENUM DEFINITIONS
//============================================================
enum
{
	WINFILE_FILE = 0,
	WINFILE_SOCKET,
	WINFILE_PTTY
};

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_file
{
	HANDLE      handle;
	SOCKET      socket;
	int         type;
	TCHAR       filename[1];
};

//============================================================
//  PROTOTYPES
//============================================================

bool win_init_sockets();
void win_cleanup_sockets();

bool win_check_socket_path(const char *path);
file_error win_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error win_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error win_write_socket(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error win_close_socket(osd_file *file);

file_error win_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error win_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error win_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error win_close_ptty(osd_file *file);

file_error win_error_to_mame_file_error(DWORD error);

#endif //__WINFILE__
