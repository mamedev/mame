// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdlfile.h - SDL file access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================


#include "osdfile.h"

#include <cstdint>
#include <string>


//============================================================
//  PROTOTYPES
//============================================================

bool posix_check_socket_path(std::string const &path);
osd_file::error posix_open_socket(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize);

bool posix_check_domain_path(std::string const &path);
osd_file::error posix_open_domain(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize);

bool posix_check_ptty_path(std::string const &path);
osd_file::error posix_open_ptty(std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize, std::string &name);

osd_file::error errno_to_file_error(int error);
