// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlfile.h - SDL file access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================
#ifndef MAME_OSD_MODULES_FILE_POSIXFILE_H
#define MAME_OSD_MODULES_FILE_POSIXFILE_H

#pragma once

#include "osdfile.h"

#include <cstdint>
#include <string>
#include <system_error>


//============================================================
//  PROTOTYPES
//============================================================

bool posix_check_socket_path(std::string const &path) noexcept;
std::error_condition posix_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept;

bool posix_check_domain_path(std::string const &path) noexcept;
std::error_condition posix_open_domain(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept;

bool posix_check_ptty_path(std::string const &path) noexcept;
std::error_condition posix_open_ptty(std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize, std::string &name) noexcept;

#endif // MAME_OSD_MODULES_FILE_POSIXFILE_H
