// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  winfile.h - File access functions
//
//============================================================
#ifndef MAME_OSD_MODULES_FILE_WINFILE_H
#define MAME_OSD_MODULES_FILE_WINFILE_H

#pragma once

#include "osdfile.h"

#include <cstdint>
#include <string>
#include <system_error>

#include <winsock2.h>


//============================================================
//  PROTOTYPES
//============================================================

bool win_init_sockets() noexcept;
void win_cleanup_sockets() noexcept;

bool win_check_socket_path(std::string const &path) noexcept;
std::error_condition win_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept;

bool win_check_ptty_path(std::string const &path) noexcept;
std::error_condition win_open_ptty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept;

std::error_condition win_error_to_file_error(DWORD error) noexcept;

#endif // MAME_OSD_MODULES_FILE_WINFILE_H
