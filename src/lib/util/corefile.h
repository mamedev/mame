// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    corefile.h

    Core file I/O interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_COREFILE_H
#define MAME_LIB_UTIL_COREFILE_H

#include "corestr.h"
#include "coretmpl.h"
#include "strformat.h"

#include <cstdint>
#include <memory>
#include <string>


namespace util {
/***************************************************************************
    ADDITIONAL OPEN FLAGS
***************************************************************************/

#define OPEN_FLAG_NO_BOM        0x0100      /* don't output BOM */

#define FCOMPRESS_NONE          0           /* no compression */
#define FCOMPRESS_MIN           1           /* minimal compression */
#define FCOMPRESS_MEDIUM        6           /* standard compression */
#define FCOMPRESS_MAX           9           /* maximum compression */


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class core_file
{
public:
	typedef std::unique_ptr<core_file> ptr;


	// ----- file open/close -----

	// open a file with the specified filename
	static osd_file::error open(std::string const &filename, std::uint32_t openflags, ptr &file);

	// open a RAM-based "file" using the given data and length (read-only)
	static osd_file::error open_ram(const void *data, std::size_t length, std::uint32_t openflags, ptr &file);

	// open a RAM-based "file" using the given data and length (read-only), copying the data
	static osd_file::error open_ram_copy(const void *data, std::size_t length, std::uint32_t openflags, ptr &file);

	// open a proxy "file" that forwards requests to another file object
	static osd_file::error open_proxy(core_file &file, ptr &proxy);

	// close an open file
	virtual ~core_file();

	// enable/disable streaming file compression via zlib; level is 0 to disable compression, or up to 9 for max compression
	virtual osd_file::error compress(int level) = 0;


	// ----- file positioning -----

	// adjust the file pointer within the file
	virtual int seek(std::int64_t offset, int whence) = 0;

	// return the current file pointer
	virtual std::uint64_t tell() const = 0;

	// return true if we are at the EOF
	virtual bool eof() const = 0;

	// return the total size of the file
	virtual std::uint64_t size() const = 0;


	// ----- file read -----

	// standard binary read from a file
	virtual std::uint32_t read(void *buffer, std::uint32_t length) = 0;

	// read one character from the file
	virtual int getc() = 0;

	// put back one character from the file
	virtual int ungetc(int c) = 0;

	// read a full line of text from the file
	virtual char *gets(char *s, int n) = 0;

	// get a pointer to a buffer that holds the full file data in RAM
	// this function may cause the full file data to be read
	virtual const void *buffer() = 0;

	// open a file with the specified filename, read it into memory, and return a pointer
	static osd_file::error load(std::string const &filename, void **data, std::uint32_t &length);
	static osd_file::error load(std::string const &filename, dynamic_buffer &data);


	// ----- file write -----

	// standard binary write to a file
	virtual std::uint32_t write(const void *buffer, std::uint32_t length) = 0;

	// write a line of text to the file
	virtual int puts(const char *s) = 0;

	// printf-style text write to a file
	virtual int vprintf(util::format_argument_pack<std::ostream> const &args) = 0;
	template <typename Format, typename... Params> int printf(Format &&fmt, Params &&...args)
	{
		return vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	// file truncation
	virtual osd_file::error truncate(std::uint64_t offset) = 0;

	// flush file buffers
	virtual osd_file::error flush() = 0;


protected:
	core_file();
};

} // namespace util


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- filename utilities ----- */

/* extract the base part of a filename (remove extensions and paths) */
std::string core_filename_extract_base(const char *name, bool strip_extension = false);

/* true if the given filename ends with a particular extension */
int core_filename_ends_with(const char *filename, const char *extension);


#endif // MAME_LIB_UTIL_COREFILE_H
