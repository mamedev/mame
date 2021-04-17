// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.h

    archive file management.

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_UNZIP_H
#define MAME_LIB_UTIL_UNZIP_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>


namespace util {
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// describes an open archive file
class archive_file
{
public:

	// Error types
	enum class error
	{
		NONE = 0,
		OUT_OF_MEMORY,
		FILE_ERROR,
		BAD_SIGNATURE,
		DECOMPRESS_ERROR,
		FILE_TRUNCATED,
		FILE_CORRUPT,
		UNSUPPORTED,
		BUFFER_TOO_SMALL
	};

	typedef std::unique_ptr<archive_file> ptr;


	/* ----- archive file access ----- */

	// open a ZIP file and parse its central directory
	static error open_zip(const std::string &filename, ptr &zip);

	// open a 7Z file and parse its central directory
	static error open_7z(const std::string &filename, ptr &result);

	// close an archive file (may actually be left open due to caching)
	virtual ~archive_file();

	// clear out all open files from the cache
	static void cache_clear();


	/* ----- contained file access ----- */

	// iterating over files - returns negative on reaching end
	virtual int first_file() = 0;
	virtual int next_file() = 0;

	// find a file index by crc, filename or both - returns non-negative on match
	virtual int search(std::uint32_t crc) = 0;
	virtual int search(const std::string &filename, bool partialpath) = 0;
	virtual int search(std::uint32_t crc, const std::string &filename, bool partialpath) = 0;

	// information on most recently found file
	virtual bool current_is_directory() const = 0;
	virtual const std::string &current_name() const = 0;
	virtual std::uint64_t current_uncompressed_length() const = 0;
	virtual std::chrono::system_clock::time_point current_last_modified() const = 0;
	virtual std::uint32_t current_crc() const = 0;

	// decompress the most recently found file in the ZIP
	virtual error decompress(void *buffer, std::uint32_t length) = 0;
};

} // namespace util

#endif  // MAME_LIB_UTIL_UNZIP_H
