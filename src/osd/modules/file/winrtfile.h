// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winfile.h - File access functions
//
//============================================================
#ifndef MAME_OSD_WINDOWS_WINFILE_H
#define MAME_OSD_WINDOWS_WINFILE_H

#include "osdfile.h"

#include <cstdint>
#include <string>

#include <winsock2.h>


//============================================================
//  PROTOTYPES
//============================================================

bool win_init_sockets();
void win_cleanup_sockets();

bool win_check_socket_path(std::string const &path);
osd_file::error win_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize);

bool win_check_ptty_path(std::string const &path);
osd_file::error win_open_ptty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize);

osd_file::error win_error_to_file_error(DWORD error);

#endif // MAME_OSD_WINDOWS_WINFILE_H
