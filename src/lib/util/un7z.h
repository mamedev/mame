// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    un7z.h

    7z file management.

***************************************************************************/

// this is based on unzip.h, with modifications needed to use the 7zip library

#pragma once

#ifndef MAME_LIB_UTIL_UN7Z_H
#define MAME_LIB_UTIL_UN7Z_H

#include "osdcore.h"

#include <cstdint>
#include <memory>
#include <string>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// describes an open _7Z file
class  _7z_file
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

	typedef  std::unique_ptr<_7z_file> ptr;


	virtual ~_7z_file();

	/* ----- 7Z file access ----- */

	// open a 7Z file and parse its central directory
	static error open(const std::string &filename, ptr &result);

	// clear out all open 7Z files from the cache
	static void cache_clear();


	/* ----- contained file access ----- */

	// iterating over files
	virtual int first_file() = 0;
	virtual int next_file() = 0;

	// find a file index by crc, filename or both
	virtual int search(std::uint32_t crc) = 0;
	virtual int search(const std::string &filename) = 0;
	virtual int search(std::uint32_t crc, const std::string &filename) = 0;

	// information on most recently found file
	virtual const std::string &current_name() const = 0;
	virtual std::uint64_t current_uncompressed_length() const = 0;
	virtual std::uint32_t current_crc() const = 0;

	// decompress the most recently found file in the _7Z
	virtual error decompress(void *buffer, std::uint32_t length) = 0;

};

#endif  // MAME_LIB_UTIL_UN7Z_H
