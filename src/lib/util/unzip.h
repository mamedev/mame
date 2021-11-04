// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.h

    archive file management.

***************************************************************************/
#ifndef MAME_LIB_UTIL_UNZIP_H
#define MAME_LIB_UTIL_UNZIP_H

#pragma once

#include "utilfwd.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>


namespace util {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// describes an open archive file
class archive_file
{
public:

	// Error types
	enum class error : int
	{
		BAD_SIGNATURE = 1,
		DECOMPRESS_ERROR,
		FILE_TRUNCATED,
		FILE_CORRUPT,
		UNSUPPORTED,
		BUFFER_TOO_SMALL
	};

	typedef std::unique_ptr<archive_file> ptr;


	/* ----- archive file access ----- */

	// open a ZIP file and parse its central directory
	static std::error_condition open_zip(std::string_view filename, ptr &result) noexcept;
	static std::error_condition open_zip(std::unique_ptr<random_read> &&file, ptr &result) noexcept;

	// open a 7Z file and parse its central directory
	static std::error_condition open_7z(std::string_view filename, ptr &result) noexcept;
	static std::error_condition open_7z(std::unique_ptr<random_read> &&file, ptr &result) noexcept;

	// close an archive file (may actually be left open due to caching)
	virtual ~archive_file();

	// clear out all open files from the cache
	static void cache_clear() noexcept;


	/* ----- contained file access ----- */

	// iterating over files - returns negative on reaching end
	virtual int first_file() noexcept = 0;
	virtual int next_file() noexcept = 0;

	// find a file index by crc, filename or both - returns non-negative on match
	virtual int search(std::uint32_t crc) noexcept = 0;
	virtual int search(std::string_view filename, bool partialpath) noexcept = 0;
	virtual int search(std::uint32_t crc, std::string_view filename, bool partialpath) noexcept = 0;

	// information on most recently found file
	virtual bool current_is_directory() const noexcept = 0;
	virtual const std::string &current_name() const noexcept = 0;
	virtual std::uint64_t current_uncompressed_length() const noexcept = 0;
	virtual std::chrono::system_clock::time_point current_last_modified() const noexcept = 0;
	virtual std::uint32_t current_crc() const noexcept = 0;

	// decompress the most recently found file in the ZIP
	virtual std::error_condition decompress(void *buffer, std::size_t length) noexcept = 0;
};


// error category for archive errors
std::error_category const &archive_category() noexcept;
inline std::error_condition make_error_condition(archive_file::error err) noexcept { return std::error_condition(int(err), archive_category()); }

} // namespace util


namespace std {

template <> struct is_error_condition_enum<util::archive_file::error> : public std::true_type { };

} // namespace std

#endif  // MAME_LIB_UTIL_UNZIP_H
