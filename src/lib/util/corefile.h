// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    corefile.h

    Core file I/O interface functions and definitions.

***************************************************************************/
#ifndef MAME_LIB_UTIL_COREFILE_H
#define MAME_LIB_UTIL_COREFILE_H

#pragma once

#include "ioprocs.h"
#include "osdfile.h"
#include "strformat.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>


namespace util {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class core_file : public random_read_write
{
public:
	using ptr = std::unique_ptr<core_file>;


	// ----- file open/close -----

	// open a file with the specified filename
	static std::error_condition open(std::string_view filename, std::uint32_t openflags, ptr &file) noexcept;

	// open a RAM-based "file" using the given data and length (read-only)
	static std::error_condition open_ram(const void *data, std::size_t length, std::uint32_t openflags, ptr &file) noexcept;

	// open a RAM-based "file" using the given data and length (read-only), copying the data
	static std::error_condition open_ram_copy(const void *data, std::size_t length, std::uint32_t openflags, ptr &file) noexcept;

	// open a proxy "file" that forwards requests to another file object
	static std::error_condition open_proxy(core_file &file, ptr &proxy) noexcept;

	// close an open file
	virtual ~core_file();


	// ----- file positioning -----

	// return true if we are at the EOF
	virtual bool eof() const = 0;


	// ----- file read -----

	// read one character from the file
	virtual int getc() = 0;

	// put back one character from the file
	virtual int ungetc(int c) = 0;

	// read a full line of text from the file
	virtual char *gets(char *s, int n) = 0;


	// file truncation
	virtual std::error_condition truncate(std::uint64_t offset) = 0;


	// open a file with the specified filename, read it into memory, and return a pointer
	static std::error_condition load(std::string_view filename, void **data, std::size_t &length) noexcept;
	static std::error_condition load(std::string_view filename, std::vector<uint8_t> &data) noexcept;
};

} // namespace util


#endif // MAME_LIB_UTIL_COREFILE_H
