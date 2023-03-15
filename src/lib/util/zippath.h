// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    zippath.h

    File/directory/path operations that work with ZIP files

***************************************************************************/

#ifndef MAME_LIB_UTIL_ZIPPATH_H
#define MAME_LIB_UTIL_ZIPPATH_H

#pragma once

#include "corefile.h"
#include "unzip.h"

#include <string>
#include <string_view>
#include <system_error>


namespace util {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class zippath_directory
{
public:
	typedef std::unique_ptr<zippath_directory> ptr;

	// opens a directory
	static std::error_condition open(std::string_view path, ptr &directory);

	// closes a directory
	virtual ~zippath_directory();

	// reads a directory entry
	virtual osd::directory::entry const *readdir() = 0;

	// returns true if this directory is an archive or false if it is a filesystem directory
	virtual bool is_archive() const = 0;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ----- path operations -----

// retrieves the parent directory
std::string zippath_parent(std::string_view path);

// combines two paths
std::string &zippath_combine(std::string &dst, const std::string &path1, const std::string &path2);
std::string zippath_combine(const std::string &path1, const std::string &path2);


// ----- file operations -----

// opens a zip path file
std::error_condition zippath_fopen(std::string_view filename, uint32_t openflags, util::core_file::ptr &file, std::string &revised_path);

} // namespace util

#endif // MAME_LIB_UTIL_ZIPPATH_H
