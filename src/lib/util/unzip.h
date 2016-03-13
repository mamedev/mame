// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.h

    ZIP file management.

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_UNZIP_H
#define MAME_LIB_UTIL_UNZIP_H

#include "osdcore.h"

#include <cstdint>
#include <memory>
#include <string>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// describes an open ZIP file
class zip_file
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

	// contains extracted file header information
	struct file_header
	{
		std::uint32_t   signature;              // central file header signature
		std::uint16_t   version_created;        // version made by
		std::uint16_t   version_needed;         // version needed to extract
		std::uint16_t   bit_flag;               // general purpose bit flag
		std::uint16_t   compression;            // compression method
		std::uint16_t   file_time;              // last mod file time
		std::uint16_t   file_date;              // last mod file date
		std::uint32_t   crc;                    // crc-32
		std::uint32_t   compressed_length;      // compressed size
		std::uint32_t   uncompressed_length;    // uncompressed size
		std::uint16_t   filename_length;        // filename length
		std::uint16_t   extra_field_length;     // extra field length
		std::uint16_t   file_comment_length;    // file comment length
		std::uint16_t   start_disk_number;      // disk number start
		std::uint16_t   internal_attributes;    // internal file attributes
		std::uint32_t   external_attributes;    // external file attributes
		std::uint32_t   local_header_offset;    // relative offset of local header
		const char *    filename;               // filename
	};

	typedef std::unique_ptr<zip_file> ptr;


	/* ----- ZIP file access ----- */

	// open a ZIP file and parse its central directory
	static error open(const std::string &filename, ptr &zip);

	// close a ZIP file (may actually be left open due to caching)
	virtual ~zip_file();

	// clear out all open ZIP files from the cache
	static void cache_clear();


	/* ----- contained file access ----- */

	// find the first file in the ZIP
	virtual const file_header *first_file() = 0;

	// find the next file in the ZIP
	virtual const file_header *next_file() = 0;

	// decompress the most recently found file in the ZIP
	virtual error decompress(void *buffer, std::uint32_t length) = 0;
};


#endif  // MAME_LIB_UTIL_UNZIP_H
