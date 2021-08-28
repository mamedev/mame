// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    chd.c

    MAME Compressed Hunks of Data file format

***************************************************************************/

#include "chd.h"

#include "avhuff.h"
#include "cdrom.h"
#include "corefile.h"
#include "coretmpl.h"
#include "flac.h"
#include "hashing.h"

#include "eminline.h"

#include <zlib.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <new>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// standard metadata formats
const char *HARD_DISK_METADATA_FORMAT = "CYLS:%d,HEADS:%d,SECS:%d,BPS:%d";
const char *CDROM_TRACK_METADATA_FORMAT = "TRACK:%d TYPE:%s SUBTYPE:%s FRAMES:%d";
const char *CDROM_TRACK_METADATA2_FORMAT = "TRACK:%d TYPE:%s SUBTYPE:%s FRAMES:%d PREGAP:%d PGTYPE:%s PGSUB:%s POSTGAP:%d";
const char *GDROM_TRACK_METADATA_FORMAT = "TRACK:%d TYPE:%s SUBTYPE:%s FRAMES:%d PAD:%d PREGAP:%d PGTYPE:%s PGSUB:%s POSTGAP:%d";
const char *AV_METADATA_FORMAT = "FPS:%d.%06d WIDTH:%d HEIGHT:%d INTERLACED:%d CHANNELS:%d SAMPLERATE:%d";

static const uint32_t METADATA_HEADER_SIZE = 16;          // metadata header size

static const uint8_t V34_MAP_ENTRY_FLAG_TYPE_MASK = 0x0f;     // what type of hunk
static const uint8_t V34_MAP_ENTRY_FLAG_NO_CRC = 0x10;        // no CRC is present



// V3-V4 entry types
enum
{
	V34_MAP_ENTRY_TYPE_INVALID = 0,             // invalid type
	V34_MAP_ENTRY_TYPE_COMPRESSED = 1,          // standard compression
	V34_MAP_ENTRY_TYPE_UNCOMPRESSED = 2,        // uncompressed data
	V34_MAP_ENTRY_TYPE_MINI = 3,                // mini: use offset as raw data
	V34_MAP_ENTRY_TYPE_SELF_HUNK = 4,           // same as another hunk in this file
	V34_MAP_ENTRY_TYPE_PARENT_HUNK = 5,         // same as a hunk in the parent file
	V34_MAP_ENTRY_TYPE_2ND_COMPRESSED = 6       // compressed with secondary algorithm (usually FLAC CDDA)
};

// V5 compression types
enum
{
	///< codec #0
	// these types are live when running
	COMPRESSION_TYPE_0 = 0,
	///< codec #1
	COMPRESSION_TYPE_1 = 1,
	///< codec #2
	COMPRESSION_TYPE_2 = 2,
	///< codec #3
	COMPRESSION_TYPE_3 = 3,
	///< no compression; implicit length = hunkbytes
	COMPRESSION_NONE = 4,
	///< same as another block in this chd
	COMPRESSION_SELF = 5,
	///< same as a hunk's worth of units in the parent chd
	COMPRESSION_PARENT = 6,

	///< start of small RLE run (4-bit length)
	// these additional pseudo-types are used for compressed encodings:
	COMPRESSION_RLE_SMALL,
	///< start of large RLE run (8-bit length)
	COMPRESSION_RLE_LARGE,
	///< same as the last COMPRESSION_SELF block
	COMPRESSION_SELF_0,
	///< same as the last COMPRESSION_SELF block + 1
	COMPRESSION_SELF_1,
	///< same block in the parent
	COMPRESSION_PARENT_SELF,
	///< same as the last COMPRESSION_PARENT block
	COMPRESSION_PARENT_0,
	///< same as the last COMPRESSION_PARENT block + 1
	COMPRESSION_PARENT_1
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> metadata_entry

// description of where a metadata entry lives within the file
struct chd_file::metadata_entry
{
	uint64_t                  offset;         // offset within the file of the header
	uint64_t                  next;           // offset within the file of the next header
	uint64_t                  prev;           // offset within the file of the previous header
	uint32_t                  length;         // length of the metadata
	uint32_t                  metatag;        // metadata tag
	uint8_t                   flags;          // flag bits
};


// ======================> metadata_hash

struct chd_file::metadata_hash
{
	uint8_t                   tag[4];         // tag of the metadata in big-endian
	util::sha1_t            sha1;           // hash data
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  be_read - extract a big-endian number from
//  a byte buffer
//-------------------------------------------------

inline uint64_t chd_file::be_read(const uint8_t *base, int numbytes)
{
	uint64_t result = 0;
	while (numbytes--)
		result = (result << 8) | *base++;
	return result;
}


//-------------------------------------------------
//  be_write - write a big-endian number to a byte
//  buffer
//-------------------------------------------------

inline void chd_file::be_write(uint8_t *base, uint64_t value, int numbytes)
{
	base += numbytes;
	while (numbytes--)
	{
		*--base = value;
		value >>= 8;
	}
}


//-------------------------------------------------
//  be_read_sha1 - fetch a sha1_t from a data
//  stream in bigendian order
//-------------------------------------------------

inline util::sha1_t chd_file::be_read_sha1(const uint8_t *base)
{
	util::sha1_t result;
	memcpy(&result.m_raw[0], base, sizeof(result.m_raw));
	return result;
}


//-------------------------------------------------
//  be_write_sha1 - write a sha1_t to a data
//  stream in bigendian order
//-------------------------------------------------

inline void chd_file::be_write_sha1(uint8_t *base, util::sha1_t value)
{
	memcpy(base, &value.m_raw[0], sizeof(value.m_raw));
}


//-------------------------------------------------
//  file_read - read from the file at the given
//  offset; on failure throw an error
//-------------------------------------------------

inline void chd_file::file_read(uint64_t offset, void *dest, uint32_t length)
{
	// no file = failure
	if (!m_file)
		throw std::error_condition(error::NOT_OPEN);

	// seek and read
	m_file->seek(offset, SEEK_SET);
	size_t count;
	std::error_condition err = m_file->read(dest, length, count);
	if (err)
		throw err;
	else if (count != length)
		throw std::error_condition(std::errc::io_error); // TODO: revisit this error code (happens if file is cut off)
}


//-------------------------------------------------
//  file_write - write to the file at the given
//  offset; on failure throw an error
//-------------------------------------------------

inline void chd_file::file_write(uint64_t offset, const void *source, uint32_t length)
{
	// no file = failure
	if (!m_file)
		throw std::error_condition(error::NOT_OPEN);

	// seek and write
	m_file->seek(offset, SEEK_SET);
	size_t count;
	std::error_condition err = m_file->write(source, length, count);
	if (err)
		throw err;
	else if (count != length)
		throw std::error_condition(std::errc::interrupted); // can theoretically happen if write is inuterrupted by a signal
}


//-------------------------------------------------
//  file_append - append to the file at the given
//  offset, ensuring we start at the given
//  alignment; on failure throw an error
//-------------------------------------------------

inline uint64_t chd_file::file_append(const void *source, uint32_t length, uint32_t alignment)
{
	std::error_condition err;

	// no file = failure
	if (!m_file)
		throw std::error_condition(error::NOT_OPEN);

	// seek to the end and align if necessary
	err = m_file->seek(0, SEEK_END);
	if (err)
		throw err;
	if (alignment != 0)
	{
		uint64_t offset;
		err = m_file->tell(offset);
		if (err)
			throw err;
		uint32_t delta = offset % alignment;
		if (delta != 0)
		{
			// pad with 0's from a local buffer
			uint8_t buffer[1024];
			memset(buffer, 0, sizeof(buffer));
			delta = alignment - delta;
			while (delta != 0)
			{
				uint32_t bytes_to_write = std::min<std::size_t>(sizeof(buffer), delta);
				size_t count;
				err = m_file->write(buffer, bytes_to_write, count);
				if (err)
					throw err;
				delta -= count;
			}
		}
	}

	// write the real data
	uint64_t offset;
	err = m_file->tell(offset);
	if (err)
		throw err;
	size_t count;
	err = m_file->write(source, length, count);
	if (err)
		throw err;
	else if (count != length)
		throw std::error_condition(std::errc::interrupted); // can theoretically happen if write is interrupted by a signal
	return offset;
}


//-------------------------------------------------
//  bits_for_value - return the number of bits
//  necessary to represent all numbers 0..value
//-------------------------------------------------

inline uint8_t chd_file::bits_for_value(uint64_t value)
{
	uint8_t result = 0;
	while (value != 0)
		value >>= 1, result++;
	return result;
}



//**************************************************************************
//  CHD FILE MANAGEMENT
//**************************************************************************

/**
 * @fn  chd_file::chd_file()
 *
 * @brief   -------------------------------------------------
 *            chd_file - constructor
 *          -------------------------------------------------.
 */

chd_file::chd_file()
{
	// reset state
	close();
}

/**
 * @fn  chd_file::~chd_file()
 *
 * @brief   -------------------------------------------------
 *            ~chd_file - destructor
 *          -------------------------------------------------.
 */

chd_file::~chd_file()
{
	// close any open files
	close();
}

/**
 * @fn  util::random_read chd_file::file()
 *
 * @brief   -------------------------------------------------
 *            file - return our underlying file
 *          -------------------------------------------------.
 *
 * @return  A random_read.
 */

util::random_read &chd_file::file()
{
	assert(m_file);
	return *m_file;
}

/**
 * @fn  util::sha1_t chd_file::sha1()
 *
 * @brief   -------------------------------------------------
 *            sha1 - return our SHA1 value
 *          -------------------------------------------------.
 *
 * @return  A sha1_t.
 */

util::sha1_t chd_file::sha1()
{
	try
	{
		// read the big-endian version
		uint8_t rawbuf[sizeof(util::sha1_t)];
		file_read(m_sha1_offset, rawbuf, sizeof(rawbuf));
		return be_read_sha1(rawbuf);
	}
	catch (std::error_condition const &)
	{
		// on failure, return nullptr
		return util::sha1_t::null;
	}
}

/**
 * @fn  util::sha1_t chd_file::raw_sha1()
 *
 * @brief   -------------------------------------------------
 *            raw_sha1 - return our raw SHA1 value
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_UNSUPPORTED_VERSION  Thrown when a chderr unsupported version error
 *                                          condition occurs.
 *
 * @return  A sha1_t.
 */

util::sha1_t chd_file::raw_sha1()
{
	try
	{
		// determine offset within the file for data-only
		if (!m_rawsha1_offset)
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// read the big-endian version
		uint8_t rawbuf[sizeof(util::sha1_t)];
		file_read(m_rawsha1_offset, rawbuf, sizeof(rawbuf));
		return be_read_sha1(rawbuf);
	}
	catch (std::error_condition const &)
	{
		// on failure, return nullptr
		return util::sha1_t::null;
	}
}

/**
 * @fn  util::sha1_t chd_file::parent_sha1()
 *
 * @brief   -------------------------------------------------
 *            parent_sha1 - return our parent's SHA1 value
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_UNSUPPORTED_VERSION  Thrown when a chderr unsupported version error
 *                                          condition occurs.
 *
 * @return  A sha1_t.
 */

util::sha1_t chd_file::parent_sha1()
{
	try
	{
		// determine offset within the file
		if (!m_parentsha1_offset)
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// read the big-endian version
		uint8_t rawbuf[sizeof(util::sha1_t)];
		file_read(m_parentsha1_offset, rawbuf, sizeof(rawbuf));
		return be_read_sha1(rawbuf);
	}
	catch (std::error_condition const &)
	{
		// on failure, return nullptr
		return util::sha1_t::null;
	}
}

/**
 * @fn  std::error_condition chd_file::hunk_info(uint32_t hunknum, chd_codec_type &compressor, uint32_t &compbytes)
 *
 * @brief   -------------------------------------------------
 *            hunk_info - return information about this hunk
 *          -------------------------------------------------.
 *
 * @param   hunknum             The hunknum.
 * @param [in,out]  compressor  The compressor.
 * @param [in,out]  compbytes   The compbytes.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::hunk_info(uint32_t hunknum, chd_codec_type &compressor, uint32_t &compbytes)
{
	// error if invalid
	if (hunknum >= m_hunkcount)
		return std::error_condition(error::HUNK_OUT_OF_RANGE);

	// get the map pointer
	uint8_t *rawmap;
	switch (m_version)
	{
		// v3/v4 map entries
		case 3:
		case 4:
			rawmap = &m_rawmap[16 * hunknum];
			switch (rawmap[15] & V34_MAP_ENTRY_FLAG_TYPE_MASK)
			{
				case V34_MAP_ENTRY_TYPE_COMPRESSED:
					compressor = CHD_CODEC_ZLIB;
					compbytes = be_read(&rawmap[12], 2) + (rawmap[14] << 16);
					break;

				case V34_MAP_ENTRY_TYPE_UNCOMPRESSED:
					compressor = CHD_CODEC_NONE;
					compbytes = m_hunkbytes;
					break;

				case V34_MAP_ENTRY_TYPE_MINI:
					compressor = CHD_CODEC_MINI;
					compbytes = 0;
					break;

				case V34_MAP_ENTRY_TYPE_SELF_HUNK:
					compressor = CHD_CODEC_SELF;
					compbytes = 0;
					break;

				case V34_MAP_ENTRY_TYPE_PARENT_HUNK:
					compressor = CHD_CODEC_PARENT;
					compbytes = 0;
					break;
			}
			break;

		// v5 map entries
		case 5:
			rawmap = &m_rawmap[m_mapentrybytes * hunknum];

			// uncompressed case
			if (!compressed())
			{
				if (be_read(&rawmap[0], 4) == 0)
				{
					compressor = CHD_CODEC_PARENT;
					compbytes = 0;
				}
				else
				{
					compressor = CHD_CODEC_NONE;
					compbytes = m_hunkbytes;
				}
				break;
			}

			// compressed case
			switch (rawmap[0])
			{
				case COMPRESSION_TYPE_0:
				case COMPRESSION_TYPE_1:
				case COMPRESSION_TYPE_2:
				case COMPRESSION_TYPE_3:
					compressor = m_compression[rawmap[0]];
					compbytes = be_read(&rawmap[1], 3);
					break;

				case COMPRESSION_NONE:
					compressor = CHD_CODEC_NONE;
					compbytes = m_hunkbytes;
					break;

				case COMPRESSION_SELF:
					compressor = CHD_CODEC_SELF;
					compbytes = 0;
					break;

				case COMPRESSION_PARENT:
					compressor = CHD_CODEC_PARENT;
					compbytes = 0;
					break;

				default:
					return error::UNKNOWN_COMPRESSION;
			}
			break;
	}
	return std::error_condition();
}

/**
 * @fn  void chd_file::set_raw_sha1(sha1_t rawdata)
 *
 * @brief   -------------------------------------------------
 *            set_raw_sha1 - set our SHA1 values
 *          -------------------------------------------------.
 *
 * @param   rawdata The rawdata.
 */

void chd_file::set_raw_sha1(util::sha1_t rawdata)
{
	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(rawbuf, rawdata);

	// write to the header
	uint64_t offset = (m_rawsha1_offset != 0) ? m_rawsha1_offset : m_sha1_offset;
	assert(offset != 0);
	file_write(offset, rawbuf, sizeof(rawbuf));

	// if we have a separate rawsha1_offset, update the full sha1 as well
	if (m_rawsha1_offset != 0)
		metadata_update_hash();
}

/**
 * @fn  void chd_file::set_parent_sha1(sha1_t parent)
 *
 * @brief   -------------------------------------------------
 *            set_parent_sha1 - set the parent SHA1 value
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_FILE Thrown when a chderr invalid file error condition occurs.
 *
 * @param   parent  The parent.
 */

void chd_file::set_parent_sha1(util::sha1_t parent)
{
	// if no file, fail
	if (!m_file)
		throw std::error_condition(error::INVALID_FILE);

	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(rawbuf, parent);

	// write to the header
	assert(m_parentsha1_offset != 0);
	file_write(m_parentsha1_offset, rawbuf, sizeof(rawbuf));
}

/**
 * @fn  std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, chd_codec_type compression[4])
 *
 * @brief   -------------------------------------------------
 *            create - create a new file with no parent using an existing opened file handle
 *          -------------------------------------------------.
 *
 * @param [in,out]  file    The file.
 * @param   logicalbytes    The logicalbytes.
 * @param   hunkbytes       The hunkbytes.
 * @param   unitbytes       The unitbytes.
 * @param   compression     The compression.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, chd_codec_type compression[4])
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;
	else if (!file)
		return std::errc::invalid_argument;

	// set the header parameters
	m_logicalbytes = logicalbytes;
	m_hunkbytes = hunkbytes;
	m_unitbytes = unitbytes;
	memcpy(m_compression, compression, sizeof(m_compression));
	m_parent = nullptr;

	// take ownership of the file
	m_file = std::move(file);
	return create_common();
}

/**
 * @fn  std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, chd_codec_type compression[4], chd_file &parent)
 *
 * @brief   -------------------------------------------------
 *            create - create a new file with a parent using an existing opened file handle
 *          -------------------------------------------------.
 *
 * @param [in,out]  file    The file.
 * @param   logicalbytes    The logicalbytes.
 * @param   hunkbytes       The hunkbytes.
 * @param   compression     The compression.
 * @param [in,out]  parent  The parent.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, chd_codec_type compression[4], chd_file &parent)
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;
	else if (!file)
		return std::errc::invalid_argument;

	// set the header parameters
	m_logicalbytes = logicalbytes;
	m_hunkbytes = hunkbytes;
	m_unitbytes = parent.unit_bytes();
	memcpy(m_compression, compression, sizeof(m_compression));
	m_parent = &parent;

	// take ownership of the file
	m_file = std::move(file);
	return create_common();
}

/**
 * @fn  std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, chd_codec_type compression[4])
 *
 * @brief   -------------------------------------------------
 *            create - create a new file with no parent using a filename
 *          -------------------------------------------------.
 *
 * @param   filename        Filename of the file.
 * @param   logicalbytes    The logicalbytes.
 * @param   hunkbytes       The hunkbytes.
 * @param   unitbytes       The unitbytes.
 * @param   compression     The compression.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, chd_codec_type compression[4])
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;

	// create the new file
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
	if (filerr)
		return filerr;
	util::random_read_write::ptr io = util::core_file_read_write(std::move(file));
	if (!io)
		return std::errc::not_enough_memory;

	// create the file normally, then claim the file
	std::error_condition chderr = create(std::move(io), logicalbytes, hunkbytes, unitbytes, compression);

	// if an error happened, close and delete the file
	if (chderr)
	{
		io.reset();
		osd_file::remove(std::string(filename)); // FIXME: allow osd_file to use std::string_view
	}
	return chderr;
}

/**
 * @fn  std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, chd_codec_type compression[4], chd_file &parent)
 *
 * @brief   -------------------------------------------------
 *            create - create a new file with a parent using a filename
 *          -------------------------------------------------.
 *
 * @param   filename        Filename of the file.
 * @param   logicalbytes    The logicalbytes.
 * @param   hunkbytes       The hunkbytes.
 * @param   compression     The compression.
 * @param [in,out]  parent  The parent.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, chd_codec_type compression[4], chd_file &parent)
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;

	// create the new file
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
	if (filerr)
		return filerr;
	util::random_read_write::ptr io = util::core_file_read_write(std::move(file));
	if (!io)
		return std::errc::not_enough_memory;

	// create the file normally, then claim the file
	std::error_condition chderr = create(std::move(io), logicalbytes, hunkbytes, compression, parent);

	// if an error happened, close and delete the file
	if (chderr)
	{
		io.reset();
		osd_file::remove(std::string(filename)); // FIXME: allow osd_file to use std::string_view
	}
	return chderr;
}

/**
 * @fn  std::error_condition chd_file::open(std::string_view filename, bool writeable, chd_file *parent)
 *
 * @brief   -------------------------------------------------
 *            open - open an existing file for read or read/write
 *          -------------------------------------------------.
 *
 * @param   filename        Filename of the file.
 * @param   writeable       true if writeable.
 * @param [in,out]  parent  If non-null, the parent.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::open(std::string_view filename, bool writeable, chd_file *parent)
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;

	// open the file
	const uint32_t openflags = writeable ? (OPEN_FLAG_READ | OPEN_FLAG_WRITE) : OPEN_FLAG_READ;
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, openflags, file);
	if (filerr)
		return filerr;
	util::random_read_write::ptr io = util::core_file_read_write(std::move(file));
	if (!io)
		return std::errc::not_enough_memory;

	// now open the CHD
	std::error_condition err = open(std::move(io), writeable, parent);
	if (err)
		return err;

	// we now own this file
	return err;
}

/**
 * @fn  std::error_condition chd_file::open(util::random_read_write::ptr &&file, bool writeable, chd_file *parent)
 *
 * @brief   -------------------------------------------------
 *            open - open an existing file for read or read/write
 *          -------------------------------------------------.
 *
 * @param [in,out]  file    The file.
 * @param   writeable       true if writeable.
 * @param [in,out]  parent  If non-null, the parent.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::open(util::random_read_write::ptr &&file, bool writeable, chd_file *parent)
{
	// make sure we don't already have a file open
	if (m_file)
		return error::ALREADY_OPEN;
	else if (!file)
		return std::errc::invalid_argument;

	// open the file
	m_file = std::move(file);
	m_parent = parent;
	m_cachehunk = ~0;
	return open_common(writeable);
}

/**
 * @fn  void chd_file::close()
 *
 * @brief   -------------------------------------------------
 *            close - close a CHD file for access
 *          -------------------------------------------------.
 */

void chd_file::close()
{
	// reset file characteristics
	m_file.reset();
	m_allow_reads = false;
	m_allow_writes = false;

	// reset core parameters from the header
	m_version = HEADER_VERSION;
	m_logicalbytes = 0;
	m_mapoffset = 0;
	m_metaoffset = 0;
	m_hunkbytes = 0;
	m_hunkcount = 0;
	m_unitbytes = 0;
	m_unitcount = 0;
	memset(m_compression, 0, sizeof(m_compression));
	m_parent = nullptr;
	m_parent_missing = false;

	// reset key offsets within the header
	m_mapoffset_offset = 0;
	m_metaoffset_offset = 0;
	m_sha1_offset = 0;
	m_rawsha1_offset = 0;
	m_parentsha1_offset = 0;

	// reset map information
	m_mapentrybytes = 0;
	m_rawmap.clear();

	// reset compression management
	for (auto & elem : m_decompressor)
		elem.reset();
	m_compressed.clear();

	// reset caching
	m_cache.clear();
	m_cachehunk = ~0;
}

/**
 * @fn  std::error_condition chd_file::read_hunk(uint32_t hunknum, void *buffer)
 *
 * @brief   -------------------------------------------------
 *            read - read a single hunk from the CHD file
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_NOT_OPEN             Thrown when a chderr not open error condition occurs.
 * @exception   CHDERR_HUNK_OUT_OF_RANGE    Thrown when a chderr hunk out of range error
 *                                          condition occurs.
 * @exception   CHDERR_DECOMPRESSION_ERROR  Thrown when a chderr decompression error error
 *                                          condition occurs.
 * @exception   CHDERR_REQUIRES_PARENT      Thrown when a chderr requires parent error condition
 *                                          occurs.
 * @exception   CHDERR_READ_ERROR           Thrown when a chderr read error error condition
 *                                          occurs.
 *
 * @param   hunknum         The hunknum.
 * @param [in,out]  buffer  If non-null, the buffer.
 *
 * @return  The hunk.
 */

std::error_condition chd_file::read_hunk(uint32_t hunknum, void *buffer)
{
	// wrap this for clean reporting
	try
	{
		// punt if no file
		if (!m_file)
			throw std::error_condition(error::NOT_OPEN);

		// return an error if out of range
		if (hunknum >= m_hunkcount)
			throw std::error_condition(error::HUNK_OUT_OF_RANGE);

		// get a pointer to the map entry
		uint64_t blockoffs;
		uint32_t blocklen;
		util::crc32_t blockcrc;
		uint8_t *rawmap;
		auto *dest = reinterpret_cast<uint8_t *>(buffer);
		switch (m_version)
		{
			// v3/v4 map entries
			case 3:
			case 4:
				rawmap = &m_rawmap[16 * hunknum];
				blockoffs = be_read(&rawmap[0], 8);
				blockcrc = be_read(&rawmap[8], 4);
				switch (rawmap[15] & V34_MAP_ENTRY_FLAG_TYPE_MASK)
				{
					case V34_MAP_ENTRY_TYPE_COMPRESSED:
						blocklen = be_read(&rawmap[12], 2) + (rawmap[14] << 16);
						file_read(blockoffs, &m_compressed[0], blocklen);
						m_decompressor[0]->decompress(&m_compressed[0], blocklen, dest, m_hunkbytes);
						if (!(rawmap[15] & V34_MAP_ENTRY_FLAG_NO_CRC) && dest != nullptr && util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();

					case V34_MAP_ENTRY_TYPE_UNCOMPRESSED:
						file_read(blockoffs, dest, m_hunkbytes);
						if (!(rawmap[15] & V34_MAP_ENTRY_FLAG_NO_CRC) && util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();

					case V34_MAP_ENTRY_TYPE_MINI:
						be_write(dest, blockoffs, 8);
						for (uint32_t bytes = 8; bytes < m_hunkbytes; bytes++)
							dest[bytes] = dest[bytes - 8];
						if (!(rawmap[15] & V34_MAP_ENTRY_FLAG_NO_CRC) && util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();

					case V34_MAP_ENTRY_TYPE_SELF_HUNK:
						return read_hunk(blockoffs, dest);

					case V34_MAP_ENTRY_TYPE_PARENT_HUNK:
						if (m_parent_missing)
							throw std::error_condition(error::REQUIRES_PARENT);
						return m_parent->read_hunk(blockoffs, dest);
				}
				break;

			// v5 map entries
			case 5:
				rawmap = &m_rawmap[m_mapentrybytes * hunknum];

				// uncompressed case
				if (!compressed())
				{
					blockoffs = uint64_t(be_read(rawmap, 4)) * uint64_t(m_hunkbytes);
					if (blockoffs != 0)
						file_read(blockoffs, dest, m_hunkbytes);
					else if (m_parent_missing)
						throw std::error_condition(error::REQUIRES_PARENT);
					else if (m_parent != nullptr)
						m_parent->read_hunk(hunknum, dest);
					else
						memset(dest, 0, m_hunkbytes);
					return std::error_condition();
				}

				// compressed case
				blocklen = be_read(&rawmap[1], 3);
				blockoffs = be_read(&rawmap[4], 6);
				blockcrc = be_read(&rawmap[10], 2);
				switch (rawmap[0])
				{
					case COMPRESSION_TYPE_0:
					case COMPRESSION_TYPE_1:
					case COMPRESSION_TYPE_2:
					case COMPRESSION_TYPE_3:
						file_read(blockoffs, &m_compressed[0], blocklen);
						m_decompressor[rawmap[0]]->decompress(&m_compressed[0], blocklen, dest, m_hunkbytes);
						if (!m_decompressor[rawmap[0]]->lossy() && dest != nullptr && util::crc16_creator::simple(dest, m_hunkbytes) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						if (m_decompressor[rawmap[0]]->lossy() && util::crc16_creator::simple(&m_compressed[0], blocklen) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();

					case COMPRESSION_NONE:
						file_read(blockoffs, dest, m_hunkbytes);
						if (util::crc16_creator::simple(dest, m_hunkbytes) != blockcrc)
							throw std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();

					case COMPRESSION_SELF:
						return read_hunk(blockoffs, dest);

					case COMPRESSION_PARENT:
						if (m_parent_missing)
							throw std::error_condition(error::REQUIRES_PARENT);
						return m_parent->read_bytes(uint64_t(blockoffs) * uint64_t(m_parent->unit_bytes()), dest, m_hunkbytes);
				}
				break;
		}

		// if we get here, something was wrong
		throw std::error_condition(std::errc::io_error);
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::write_hunk(uint32_t hunknum, const void *buffer)
 *
 * @brief   -------------------------------------------------
 *            write - write a single hunk to the CHD file
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_NOT_OPEN             Thrown when a chderr not open error condition occurs.
 * @exception   CHDERR_HUNK_OUT_OF_RANGE    Thrown when a chderr hunk out of range error
 *                                          condition occurs.
 * @exception   CHDERR_FILE_NOT_WRITEABLE   Thrown when a chderr file not writeable error
 *                                          condition occurs.
 *
 * @param   hunknum The hunknum.
 * @param   buffer  The buffer.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::write_hunk(uint32_t hunknum, const void *buffer)
{
	// wrap this for clean reporting
	try
	{
		// punt if no file
		if (!m_file)
			throw std::error_condition(error::NOT_OPEN);

		// return an error if out of range
		if (hunknum >= m_hunkcount)
			throw std::error_condition(error::HUNK_OUT_OF_RANGE);

		// if not writeable, fail
		if (!m_allow_writes)
			throw std::error_condition(error::FILE_NOT_WRITEABLE);

		// uncompressed writes only via this interface
		if (compressed())
			throw std::error_condition(error::FILE_NOT_WRITEABLE);

		// see if we have allocated the space on disk for this hunk
		uint8_t *rawmap = &m_rawmap[hunknum * 4];
		uint32_t rawentry = be_read(rawmap, 4);

		// if not, allocate one now
		if (rawentry == 0)
		{
			// first make sure we need to allocate it
			bool all_zeros = true;
			const auto *scan = reinterpret_cast<const uint32_t *>(buffer);
			for (uint32_t index = 0; index < m_hunkbytes / 4; index++)
				if (scan[index] != 0)
				{
					all_zeros = false;
					break;
				}

			// if it's all zeros, do nothing more
			if (all_zeros)
				return std::error_condition();

			// append new data to the end of the file, aligning the first chunk
			rawentry = file_append(buffer, m_hunkbytes, m_hunkbytes) / m_hunkbytes;

			// write the map entry back
			be_write(rawmap, rawentry, 4);
			file_write(m_mapoffset + hunknum * 4, rawmap, 4);

			// update the cached hunk if we just wrote it
			if (hunknum == m_cachehunk && buffer != &m_cache[0])
				memcpy(&m_cache[0], buffer, m_hunkbytes);
		}
		else
		{
			// otherwise, just overwrite
			file_write(uint64_t(rawentry) * uint64_t(m_hunkbytes), buffer, m_hunkbytes);
		}
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::read_units(uint64_t unitnum, void *buffer, uint32_t count)
 *
 * @brief   -------------------------------------------------
 *            read_units - read the given number of units from the CHD
 *          -------------------------------------------------.
 *
 * @param   unitnum         The unitnum.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   count           Number of.
 *
 * @return  The units.
 */

std::error_condition chd_file::read_units(uint64_t unitnum, void *buffer, uint32_t count)
{
	return read_bytes(unitnum * uint64_t(m_unitbytes), buffer, count * m_unitbytes);
}

/**
 * @fn  std::error_condition chd_file::write_units(uint64_t unitnum, const void *buffer, uint32_t count)
 *
 * @brief   -------------------------------------------------
 *            write_units - write the given number of units to the CHD
 *          -------------------------------------------------.
 *
 * @param   unitnum The unitnum.
 * @param   buffer  The buffer.
 * @param   count   Number of.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::write_units(uint64_t unitnum, const void *buffer, uint32_t count)
{
	return write_bytes(unitnum * uint64_t(m_unitbytes), buffer, count * m_unitbytes);
}

/**
 * @fn  std::error_condition chd_file::read_bytes(uint64_t offset, void *buffer, uint32_t bytes)
 *
 * @brief   -------------------------------------------------
 *            read_bytes - read from the CHD at a byte level, using the cache to handle partial
 *            hunks
 *          -------------------------------------------------.
 *
 * @param   offset          The offset.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   bytes           The bytes.
 *
 * @return  The bytes.
 */

std::error_condition chd_file::read_bytes(uint64_t offset, void *buffer, uint32_t bytes)
{
	// iterate over hunks
	uint32_t first_hunk = offset / m_hunkbytes;
	uint32_t last_hunk = (offset + bytes - 1) / m_hunkbytes;
	auto *dest = reinterpret_cast<uint8_t *>(buffer);
	for (uint32_t curhunk = first_hunk; curhunk <= last_hunk; curhunk++)
	{
		// determine start/end boundaries
		uint32_t startoffs = (curhunk == first_hunk) ? (offset % m_hunkbytes) : 0;
		uint32_t endoffs = (curhunk == last_hunk) ? ((offset + bytes - 1) % m_hunkbytes) : (m_hunkbytes - 1);

		// if it's a full block, just read directly from disk unless it's the cached hunk
		std::error_condition err;
		if (startoffs == 0 && endoffs == m_hunkbytes - 1 && curhunk != m_cachehunk)
			err = read_hunk(curhunk, dest);

		// otherwise, read from the cache
		else
		{
			if (curhunk != m_cachehunk)
			{
				err = read_hunk(curhunk, &m_cache[0]);
				if (err)
					return err;
				m_cachehunk = curhunk;
			}
			memcpy(dest, &m_cache[startoffs], endoffs + 1 - startoffs);
		}

		// handle errors and advance
		if (err)
			return err;
		dest += endoffs + 1 - startoffs;
	}
	return std::error_condition();
}

/**
 * @fn  std::error_condition chd_file::write_bytes(uint64_t offset, const void *buffer, uint32_t bytes)
 *
 * @brief   -------------------------------------------------
 *            write_bytes - write to the CHD at a byte level, using the cache to handle partial
 *            hunks
 *          -------------------------------------------------.
 *
 * @param   offset  The offset.
 * @param   buffer  The buffer.
 * @param   bytes   The bytes.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::write_bytes(uint64_t offset, const void *buffer, uint32_t bytes)
{
	// iterate over hunks
	uint32_t first_hunk = offset / m_hunkbytes;
	uint32_t last_hunk = (offset + bytes - 1) / m_hunkbytes;
	const auto *source = reinterpret_cast<const uint8_t *>(buffer);
	for (uint32_t curhunk = first_hunk; curhunk <= last_hunk; curhunk++)
	{
		// determine start/end boundaries
		uint32_t startoffs = (curhunk == first_hunk) ? (offset % m_hunkbytes) : 0;
		uint32_t endoffs = (curhunk == last_hunk) ? ((offset + bytes - 1) % m_hunkbytes) : (m_hunkbytes - 1);

		// if it's a full block, just write directly to disk unless it's the cached hunk
		std::error_condition err;
		if (startoffs == 0 && endoffs == m_hunkbytes - 1 && curhunk != m_cachehunk)
			err = write_hunk(curhunk, source);

		// otherwise, write from the cache
		else
		{
			if (curhunk != m_cachehunk)
			{
				err = read_hunk(curhunk, &m_cache[0]);
				if (err)
					return err;
				m_cachehunk = curhunk;
			}
			memcpy(&m_cache[startoffs], source, endoffs + 1 - startoffs);
			err = write_hunk(curhunk, &m_cache[0]);
		}

		// handle errors and advance
		if (err)
			return err;
		source += endoffs + 1 - startoffs;
	}
	return std::error_condition();
}

/**
 * @fn  std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::string &output)
 *
 * @brief   -------------------------------------------------
 *            read_metadata - read the indexed metadata of the given type
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_METADATA_NOT_FOUND   Thrown when a chderr metadata not found error
 *                                          condition occurs.
 *
 * @param   searchtag       The searchtag.
 * @param   searchindex     The searchindex.
 * @param [in,out]  output  The output.
 *
 * @return  The metadata.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::string &output)
{
	// wrap this for clean reporting
	try
	{
		// if we didn't find it, just return
		metadata_entry metaentry;
		if (!metadata_find(searchtag, searchindex, metaentry))
			throw std::error_condition(error::METADATA_NOT_FOUND);

		// read the metadata
		output.assign(metaentry.length, '\0');
		file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output)
 *
 * @brief   Reads a metadata.
 *
 * @exception   CHDERR_METADATA_NOT_FOUND   Thrown when a chderr metadata not found error
 *                                          condition occurs.
 *
 * @param   searchtag       The searchtag.
 * @param   searchindex     The searchindex.
 * @param [in,out]  output  The output.
 *
 * @return  The metadata.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output)
{
	// wrap this for clean reporting
	try
	{
		// if we didn't find it, just return
		metadata_entry metaentry;
		if (!metadata_find(searchtag, searchindex, metaentry))
			throw std::error_condition(error::METADATA_NOT_FOUND);

		// read the metadata
		output.resize(metaentry.length);
		file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, void *output, uint32_t outputlen, uint32_t &resultlen)
 *
 * @brief   Reads a metadata.
 *
 * @exception   CHDERR_METADATA_NOT_FOUND   Thrown when a chderr metadata not found error
 *                                          condition occurs.
 *
 * @param   searchtag           The searchtag.
 * @param   searchindex         The searchindex.
 * @param [in,out]  output      If non-null, the output.
 * @param   outputlen           The outputlen.
 * @param [in,out]  resultlen   The resultlen.
 *
 * @return  The metadata.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, void *output, uint32_t outputlen, uint32_t &resultlen)
{
	// wrap this for clean reporting
	try
	{
		// if we didn't find it, just return
		metadata_entry metaentry;
		if (!metadata_find(searchtag, searchindex, metaentry))
			throw std::error_condition(error::METADATA_NOT_FOUND);

		// read the metadata
		resultlen = metaentry.length;
		file_read(metaentry.offset + METADATA_HEADER_SIZE, output, std::min(outputlen, resultlen));
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output, chd_metadata_tag &resulttag, uint8_t &resultflags)
 *
 * @brief   Reads a metadata.
 *
 * @exception   CHDERR_METADATA_NOT_FOUND   Thrown when a chderr metadata not found error
 *                                          condition occurs.
 *
 * @param   searchtag           The searchtag.
 * @param   searchindex         The searchindex.
 * @param [in,out]  output      The output.
 * @param [in,out]  resulttag   The resulttag.
 * @param [in,out]  resultflags The resultflags.
 *
 * @return  The metadata.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output, chd_metadata_tag &resulttag, uint8_t &resultflags)
{
	// wrap this for clean reporting
	try
	{
		// if we didn't find it, just return
		metadata_entry metaentry;
		if (!metadata_find(searchtag, searchindex, metaentry))
			throw std::error_condition(error::METADATA_NOT_FOUND);

		// read the metadata
		output.resize(metaentry.length);
		file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
		resulttag = metaentry.metatag;
		resultflags = metaentry.flags;
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::write_metadata(chd_metadata_tag metatag, uint32_t metaindex, const void *inputbuf, uint32_t inputlen, uint8_t flags)
 *
 * @brief   -------------------------------------------------
 *            write_metadata - write the indexed metadata of the given type
 *          -------------------------------------------------.
 *
 * @param   metatag     The metatag.
 * @param   metaindex   The metaindex.
 * @param   inputbuf    The inputbuf.
 * @param   inputlen    The inputlen.
 * @param   flags       The flags.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::write_metadata(chd_metadata_tag metatag, uint32_t metaindex, const void *inputbuf, uint32_t inputlen, uint8_t flags)
{
	// wrap this for clean reporting
	try
	{
		// must write at least 1 byte and no more than 16MB
		if (inputlen < 1 || inputlen >= 16 * 1024 * 1024)
			return std::error_condition(std::errc::invalid_argument);

		// find the entry if it already exists
		metadata_entry metaentry;
		bool finished = false;
		if (metadata_find(metatag, metaindex, metaentry))
		{
			// if the new data fits over the old data, just overwrite
			if (inputlen <= metaentry.length)
			{
				file_write(metaentry.offset + METADATA_HEADER_SIZE, inputbuf, inputlen);

				// if the lengths don't match, we need to update the length in our header
				if (inputlen != metaentry.length)
				{
					uint8_t length[3];
					be_write(length, inputlen, 3);
					file_write(metaentry.offset + 5, length, sizeof(length));
				}

				// indicate we did everything
				finished = true;
			}

			// if it doesn't fit, unlink the current entry
			else
				metadata_set_previous_next(metaentry.prev, metaentry.next);
		}

		// if not yet done, create a new entry and append
		if (!finished)
		{
			// now build us a new entry
			uint8_t raw_meta_header[METADATA_HEADER_SIZE];
			be_write(&raw_meta_header[0], metatag, 4);
			raw_meta_header[4] = flags;
			be_write(&raw_meta_header[5], (inputlen & 0x00ffffff) | (flags << 24), 3);
			be_write(&raw_meta_header[8], 0, 8);

			// append the new header, then the data
			uint64_t offset = file_append(raw_meta_header, sizeof(raw_meta_header));
			file_append(inputbuf, inputlen);

			// set the previous entry to point to us
			metadata_set_previous_next(metaentry.prev, offset);
		}

		// update the hash
		metadata_update_hash();
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// return any errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::delete_metadata(chd_metadata_tag metatag, uint32_t metaindex)
 *
 * @brief   -------------------------------------------------
 *            delete_metadata - remove the given metadata from the list
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_METADATA_NOT_FOUND   Thrown when a chderr metadata not found error
 *                                          condition occurs.
 *
 * @param   metatag     The metatag.
 * @param   metaindex   The metaindex.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::delete_metadata(chd_metadata_tag metatag, uint32_t metaindex)
{
	// wrap this for clean reporting
	try
	{
		// find the entry
		metadata_entry metaentry;
		if (!metadata_find(metatag, metaindex, metaentry))
			throw std::error_condition(error::METADATA_NOT_FOUND);

		// point the previous to the next, unlinking us
		metadata_set_previous_next(metaentry.prev, metaentry.next);
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// return any errors
		return err;
	}
}

/**
 * @fn  std::error_condition chd_file::clone_all_metadata(chd_file &source)
 *
 * @brief   -------------------------------------------------
 *            clone_all_metadata - clone the metadata from one CHD to a second
 *          -------------------------------------------------.
 *
 * @exception   err Thrown when an error error condition occurs.
 *
 * @param [in,out]  source  Another instance to copy.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::clone_all_metadata(chd_file &source)
{
	// wrap this for clean reporting
	try
	{
		// iterate over metadata entries in the source
		std::vector<uint8_t> filedata;
		metadata_entry metaentry;
		metaentry.metatag = 0;
		metaentry.length = 0;
		metaentry.next = 0;
		metaentry.flags = 0;
		for (bool has_data = source.metadata_find(CHDMETATAG_WILDCARD, 0, metaentry); has_data; has_data = source.metadata_find(CHDMETATAG_WILDCARD, 0, metaentry, true))
		{
			// read the metadata item
			filedata.resize(metaentry.length);
			source.file_read(metaentry.offset + METADATA_HEADER_SIZE, &filedata[0], metaentry.length);

			// write it to the destination
			std::error_condition err = write_metadata(metaentry.metatag, (uint32_t)-1, &filedata[0], metaentry.length, metaentry.flags);
			if (err)
				throw err;
		}
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// return any errors
		return err;
	}
}

/**
 * @fn  util::sha1_t chd_file::compute_overall_sha1(sha1_t rawsha1)
 *
 * @brief   -------------------------------------------------
 *            compute_overall_sha1 - iterate through the metadata and compute the overall hash of
 *            the CHD file
 *          -------------------------------------------------.
 *
 * @param   rawsha1 The first rawsha.
 *
 * @return  The calculated overall sha 1.
 */

util::sha1_t chd_file::compute_overall_sha1(util::sha1_t rawsha1)
{
	// only works for v4 and above
	if (m_version < 4)
		return rawsha1;

	// iterate over metadata
	std::vector<uint8_t> filedata;
	std::vector<metadata_hash> hasharray;
	metadata_entry metaentry;
	for (bool has_data = metadata_find(CHDMETATAG_WILDCARD, 0, metaentry); has_data; has_data = metadata_find(CHDMETATAG_WILDCARD, 0, metaentry, true))
	{
		// if not checksumming, continue
		if ((metaentry.flags & CHD_MDFLAGS_CHECKSUM) == 0)
			continue;

		// allocate memory and read the data
		filedata.resize(metaentry.length);
		file_read(metaentry.offset + METADATA_HEADER_SIZE, &filedata[0], metaentry.length);

		// create an entry for this metadata and add it
		metadata_hash hashentry;
		be_write(hashentry.tag, metaentry.metatag, 4);
		hashentry.sha1 = util::sha1_creator::simple(&filedata[0], metaentry.length);
		hasharray.push_back(hashentry);
	}

	// sort the array
	if (!hasharray.empty())
		qsort(&hasharray[0], hasharray.size(), sizeof(hasharray[0]), metadata_hash_compare);

	// read the raw data hash from our header and start a new SHA1 with that data
	util::sha1_creator overall_sha1;
	overall_sha1.append(&rawsha1, sizeof(rawsha1));
	if (!hasharray.empty())
		overall_sha1.append(&hasharray[0], hasharray.size() * sizeof(hasharray[0]));
	return overall_sha1.finish();
}

/**
 * @fn  std::error_condition chd_file::codec_configure(chd_codec_type codec, int param, void *config)
 *
 * @brief   -------------------------------------------------
 *            codec_config - set internal codec parameters
 *          -------------------------------------------------.
 *
 * @param   codec           The codec.
 * @param   param           The parameter.
 * @param [in,out]  config  If non-null, the configuration.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::codec_configure(chd_codec_type codec, int param, void *config)
{
	// wrap this for clean reporting
	try
	{
		// find the codec and call its configuration
		for (int codecnum = 0; codecnum < std::size(m_compression); codecnum++)
			if (m_compression[codecnum] == codec)
			{
				m_decompressor[codecnum]->configure(param, config);
				return std::error_condition();
			}
		return std::errc::invalid_argument;
	}
	catch (std::error_condition const &err)
	{
		// return any errors
		return err;
	}
}

/**
 * @fn  const char *chd_file::error_string(chd_error err)
 *
 * @brief   -------------------------------------------------
 *            error_string - return an error string for the given CHD error
 *          -------------------------------------------------.
 *
 * @param   err The error.
 *
 * @return  null if it fails, else a char*.
 */

std::error_category const &chd_category() noexcept
{
	class chd_category_impl : public std::error_category
	{
		virtual char const *name() const noexcept override { return "chd"; }

		virtual std::string message(int condition) const override
		{
			using namespace std::literals;
			static std::string_view const s_messages[] = {
					"No error"sv,
					"No drive interface"sv,
					"File not open"sv,
					"File already open"sv,
					"Invalid file"sv,
					"Invalid data"sv,
					"Requires parent"sv,
					"File not writeable"sv,
					"Codec error"sv,
					"Invalid parent"sv,
					"Hunk out of range"sv,
					"Decompression error"sv,
					"Compression error"sv,
					"Can't verify file"sv,
					"Can't find metadata"sv,
					"Invalid metadata size"sv,
					"Mismatched DIFF and CHD or unsupported CHD version"sv,
					"Incomplete verify"sv,
					"Invalid metadata"sv,
					"Invalid state"sv,
					"Operation pending"sv,
					"Unsupported format"sv,
					"Unknown compression type"sv,
					"Currently examining parent"sv,
					"Currently compressing"sv };
			if ((0 <= condition) && (std::size(s_messages) > condition))
				return std::string(s_messages[condition]);
			else
				return "Unknown error"s;
		}
	};
	static chd_category_impl const s_chd_category_instance;
	return s_chd_category_instance;
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

/**
 * @fn  uint32_t chd_file::guess_unitbytes()
 *
 * @brief   -------------------------------------------------
 *            guess_unitbytes - for older CHD formats, take a guess at the bytes/unit based on
 *            metadata
 *          -------------------------------------------------.
 *
 * @return  An uint32_t.
 */

uint32_t chd_file::guess_unitbytes()
{
	// look for hard disk metadata; if found, then the unit size == sector size
	std::string metadata;
	int i0, i1, i2, i3;
	if (!read_metadata(HARD_DISK_METADATA_TAG, 0, metadata) && sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &i0, &i1, &i2, &i3) == 4)
		return i3;

	// look for CD-ROM metadata; if found, then the unit size == CD frame size
	if (!read_metadata(CDROM_OLD_METADATA_TAG, 0, metadata) ||
		!read_metadata(CDROM_TRACK_METADATA_TAG, 0, metadata) ||
		!read_metadata(CDROM_TRACK_METADATA2_TAG, 0, metadata) ||
		!read_metadata(GDROM_OLD_METADATA_TAG, 0, metadata) ||
		!read_metadata(GDROM_TRACK_METADATA_TAG, 0, metadata))
		return CD_FRAME_SIZE;

	// otherwise, just map 1:1 with the hunk size
	return m_hunkbytes;
}

/**
 * @fn  void chd_file::parse_v3_header(uint8_t *rawheader, sha1_t &parentsha1)
 *
 * @brief   -------------------------------------------------
 *            parse_v3_header - parse the header from a v3 file and configure core parameters
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_FILE         Thrown when a chderr invalid file error condition
 *                                          occurs.
 * @exception   CHDERR_UNKNOWN_COMPRESSION  Thrown when a chderr unknown compression error
 *                                          condition occurs.
 *
 * @param [in,out]  rawheader   If non-null, the rawheader.
 * @param [in,out]  parentsha1  The first parentsha.
 */

void chd_file::parse_v3_header(uint8_t *rawheader, util::sha1_t &parentsha1)
{
	// verify header length
	if (be_read(&rawheader[8], 4) != V3_HEADER_SIZE)
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = be_read(&rawheader[28], 8);
	m_mapoffset = 120;
	m_metaoffset = be_read(&rawheader[36], 8);
	m_hunkbytes = be_read(&rawheader[76], 4);
	m_hunkcount = be_read(&rawheader[24], 4);

	// extract parent SHA-1
	uint32_t flags = be_read(&rawheader[16], 4);
	m_allow_writes = (flags & 2) == 0;

	// determine compression
	switch (be_read(&rawheader[20], 4))
	{
		case 0: m_compression[0] = CHD_CODEC_NONE;      break;
		case 1: m_compression[0] = CHD_CODEC_ZLIB;      break;
		case 2: m_compression[0] = CHD_CODEC_ZLIB;      break;
		case 3: m_compression[0] = CHD_CODEC_AVHUFF;    break;
		default: throw std::error_condition(error::UNKNOWN_COMPRESSION);
	}
	m_compression[1] = m_compression[2] = m_compression[3] = CHD_CODEC_NONE;

	// describe the format
	m_mapoffset_offset = 0;
	m_metaoffset_offset = 36;
	m_sha1_offset = 80;
	m_rawsha1_offset = 0;
	m_parentsha1_offset = 100;

	// determine properties of map entries
	m_mapentrybytes = 16;

	// extract parent SHA-1
	if (flags & 1)
		parentsha1 = be_read_sha1(&rawheader[m_parentsha1_offset]);

	// guess at the units based on snooping the metadata
	m_unitbytes = guess_unitbytes();
	m_unitcount = (m_logicalbytes + m_unitbytes - 1) / m_unitbytes;
}

/**
 * @fn  void chd_file::parse_v4_header(uint8_t *rawheader, sha1_t &parentsha1)
 *
 * @brief   -------------------------------------------------
 *            parse_v4_header - parse the header from a v4 file and configure core parameters
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_FILE         Thrown when a chderr invalid file error condition
 *                                          occurs.
 * @exception   CHDERR_UNKNOWN_COMPRESSION  Thrown when a chderr unknown compression error
 *                                          condition occurs.
 *
 * @param [in,out]  rawheader   If non-null, the rawheader.
 * @param [in,out]  parentsha1  The first parentsha.
 */

void chd_file::parse_v4_header(uint8_t *rawheader, util::sha1_t &parentsha1)
{
	// verify header length
	if (be_read(&rawheader[8], 4) != V4_HEADER_SIZE)
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = be_read(&rawheader[28], 8);
	m_mapoffset = 108;
	m_metaoffset = be_read(&rawheader[36], 8);
	m_hunkbytes = be_read(&rawheader[44], 4);
	m_hunkcount = be_read(&rawheader[24], 4);

	// extract parent SHA-1
	uint32_t flags = be_read(&rawheader[16], 4);
	m_allow_writes = (flags & 2) == 0;

	// determine compression
	switch (be_read(&rawheader[20], 4))
	{
		case 0: m_compression[0] = CHD_CODEC_NONE;      break;
		case 1: m_compression[0] = CHD_CODEC_ZLIB;      break;
		case 2: m_compression[0] = CHD_CODEC_ZLIB;      break;
		case 3: m_compression[0] = CHD_CODEC_AVHUFF;    break;
		default: throw std::error_condition(error::UNKNOWN_COMPRESSION);
	}
	m_compression[1] = m_compression[2] = m_compression[3] = CHD_CODEC_NONE;

	// describe the format
	m_mapoffset_offset = 0;
	m_metaoffset_offset = 36;
	m_sha1_offset = 48;
	m_rawsha1_offset = 88;
	m_parentsha1_offset = 68;

	// determine properties of map entries
	m_mapentrybytes = 16;

	// extract parent SHA-1
	if (flags & 1)
		parentsha1 = be_read_sha1(&rawheader[m_parentsha1_offset]);

	// guess at the units based on snooping the metadata
	m_unitbytes = guess_unitbytes();
	m_unitcount = (m_logicalbytes + m_unitbytes - 1) / m_unitbytes;
}

/**
 * @fn  void chd_file::parse_v5_header(uint8_t *rawheader, sha1_t &parentsha1)
 *
 * @brief   -------------------------------------------------
 *            parse_v5_header - read the header from a v5 file and configure core parameters
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_FILE Thrown when a chderr invalid file error condition occurs.
 *
 * @param [in,out]  rawheader   If non-null, the rawheader.
 * @param [in,out]  parentsha1  The first parentsha.
 */

void chd_file::parse_v5_header(uint8_t *rawheader, util::sha1_t &parentsha1)
{
	// verify header length
	if (be_read(&rawheader[8], 4) != V5_HEADER_SIZE)
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = be_read(&rawheader[32], 8);
	m_mapoffset = be_read(&rawheader[40], 8);
	m_metaoffset = be_read(&rawheader[48], 8);
	m_hunkbytes = be_read(&rawheader[56], 4);
	m_hunkcount = (m_logicalbytes + m_hunkbytes - 1) / m_hunkbytes;
	m_unitbytes = be_read(&rawheader[60], 4);
	m_unitcount = (m_logicalbytes + m_unitbytes - 1) / m_unitbytes;

	// determine compression
	m_compression[0] = be_read(&rawheader[16], 4);
	m_compression[1] = be_read(&rawheader[20], 4);
	m_compression[2] = be_read(&rawheader[24], 4);
	m_compression[3] = be_read(&rawheader[28], 4);

	m_allow_writes = !compressed();

	// describe the format
	m_mapoffset_offset = 40;
	m_metaoffset_offset = 48;
	m_sha1_offset = 84;
	m_rawsha1_offset = 64;
	m_parentsha1_offset = 104;

	// determine properties of map entries
	m_mapentrybytes = compressed() ? 12 : 4;

	// extract parent SHA-1
	parentsha1 = be_read_sha1(&rawheader[m_parentsha1_offset]);
}

/**
 * @fn  std::error_condition chd_file::compress_v5_map()
 *
 * @brief   -------------------------------------------------
 *            compress_v5_map - compress the v5 map and write it to the end of the file
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_COMPRESSION_ERROR    Thrown when a chderr compression error error
 *                                          condition occurs.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::compress_v5_map()
{
	try
	{
		// first get a CRC-16 of the original rawmap
		util::crc16_t mapcrc = util::crc16_creator::simple(&m_rawmap[0], m_hunkcount * 12);

		// create a buffer to hold the RLE data
		std::vector<uint8_t> compression_rle(m_hunkcount);
		uint8_t *dest = &compression_rle[0];

		// use a huffman encoder for 16 different codes, maximum length is 8 bits
		huffman_encoder<16, 8> encoder;
		encoder.histo_reset();

		// RLE-compress the compression type since we expect runs of the same
		uint32_t max_self = 0;
		uint32_t last_self = 0;
		uint64_t max_parent = 0;
		uint64_t last_parent = 0;
		uint32_t max_complen = 0;
		uint8_t lastcomp = 0;
		int count = 0;
		for (int hunknum = 0; hunknum < m_hunkcount; hunknum++)
		{
			uint8_t curcomp = m_rawmap[hunknum * 12 + 0];

			// promote self block references to more compact forms
			if (curcomp == COMPRESSION_SELF)
			{
				uint32_t refhunk = be_read(&m_rawmap[hunknum * 12 + 4], 6);
				if (refhunk == last_self)
					curcomp = COMPRESSION_SELF_0;
				else if (refhunk == last_self + 1)
					curcomp = COMPRESSION_SELF_1;
				else
					max_self = std::max(max_self, refhunk);
				last_self = refhunk;
			}

			// promote parent block references to more compact forms
			else if (curcomp == COMPRESSION_PARENT)
			{
				uint32_t refunit = be_read(&m_rawmap[hunknum * 12 + 4], 6);
				if (refunit == (uint64_t(hunknum) * uint64_t(m_hunkbytes)) / m_unitbytes)
					curcomp = COMPRESSION_PARENT_SELF;
				else if (refunit == last_parent)
					curcomp = COMPRESSION_PARENT_0;
				else if (refunit == last_parent + m_hunkbytes / m_unitbytes)
					curcomp = COMPRESSION_PARENT_1;
				else
					max_parent = std::max(max_parent, uint64_t(refunit));
				last_parent = refunit;
			}

			// track maximum compressed length
			else //if (curcomp >= COMPRESSION_TYPE_0 && curcomp <= COMPRESSION_TYPE_3)
				max_complen = std::max(max_complen, uint32_t(be_read(&m_rawmap[hunknum * 12 + 1], 3)));

			// track repeats
			if (curcomp == lastcomp)
				count++;

			// if no repeat, or we're at the end, flush it
			if (curcomp != lastcomp || hunknum == m_hunkcount - 1)
			{
				while (count != 0)
				{
					if (count < 3)
						encoder.histo_one(*dest++ = lastcomp), count--;
					else if (count <= 3+15)
					{
						encoder.histo_one(*dest++ = COMPRESSION_RLE_SMALL);
						encoder.histo_one(*dest++ = count - 3);
						count = 0;
					}
					else
					{
						int this_count = std::min(count, 3+16+255);
						encoder.histo_one(*dest++ = COMPRESSION_RLE_LARGE);
						encoder.histo_one(*dest++ = (this_count - 3 - 16) >> 4);
						encoder.histo_one(*dest++ = (this_count - 3 - 16) & 15);
						count -= this_count;
					}
				}
				if (curcomp != lastcomp)
					encoder.histo_one(*dest++ = lastcomp = curcomp);
			}
		}

		// compute a tree and export it to the buffer
		std::vector<uint8_t> compressed(m_hunkcount * 6);
		bitstream_out bitbuf(&compressed[16], compressed.size() - 16);
		huffman_error err = encoder.compute_tree_from_histo();
		if (err != HUFFERR_NONE)
			throw std::error_condition(error::COMPRESSION_ERROR);
		err = encoder.export_tree_rle(bitbuf);
		if (err != HUFFERR_NONE)
			throw std::error_condition(error::COMPRESSION_ERROR);

		// encode the data
		for (uint8_t *src = &compression_rle[0]; src < dest; src++)
			encoder.encode_one(bitbuf, *src);

		// determine the number of bits we need to hold the a length
		// and a hunk index
		uint8_t lengthbits = bits_for_value(max_complen);
		uint8_t selfbits = bits_for_value(max_self);
		uint8_t parentbits = bits_for_value(max_parent);

		// for each compression type, output the relevant data
		lastcomp = 0;
		count = 0;
		uint8_t *src = &compression_rle[0];
		uint64_t firstoffs = 0;
		for (int hunknum = 0; hunknum < m_hunkcount; hunknum++)
		{
			uint8_t *rawmap = &m_rawmap[hunknum * 12];
			uint32_t length = be_read(&rawmap[1], 3);
			uint64_t offset = be_read(&rawmap[4], 6);
			uint16_t crc = be_read(&rawmap[10], 2);

			// if no count remaining, fetch the next entry
			if (count == 0)
			{
				uint8_t val = *src++;
				if (val == COMPRESSION_RLE_SMALL)
					count = 2 + *src++;
				else if (val == COMPRESSION_RLE_LARGE)
					count = 2 + 16 + (*src++ << 4), count += *src++;
				else
					lastcomp = val;
			}
			else
				count--;

			// output additional data needed for this entry
			switch (lastcomp)
			{
				case COMPRESSION_TYPE_0:
				case COMPRESSION_TYPE_1:
				case COMPRESSION_TYPE_2:
				case COMPRESSION_TYPE_3:
					assert(length < (1 << lengthbits));
					bitbuf.write(length, lengthbits);
					bitbuf.write(crc, 16);
					if (firstoffs == 0)
						firstoffs = offset;
					break;

				case COMPRESSION_NONE:
					bitbuf.write(crc, 16);
					if (firstoffs == 0)
						firstoffs = offset;
					break;

				case COMPRESSION_SELF:
					assert(offset < (uint64_t(1) << selfbits));
					bitbuf.write(offset, selfbits);
					break;

				case COMPRESSION_PARENT:
					assert(offset < (uint64_t(1) << parentbits));
					bitbuf.write(offset, parentbits);
					break;

				case COMPRESSION_SELF_0:
				case COMPRESSION_SELF_1:
				case COMPRESSION_PARENT_SELF:
				case COMPRESSION_PARENT_0:
				case COMPRESSION_PARENT_1:
					break;
			}
		}

		// write the map header
		uint32_t complen = bitbuf.flush();
		assert(!bitbuf.overflow());
		be_write(&compressed[0], complen, 4);
		be_write(&compressed[4], firstoffs, 6);
		be_write(&compressed[10], mapcrc, 2);
		compressed[12] = lengthbits;
		compressed[13] = selfbits;
		compressed[14] = parentbits;
		compressed[15] = 0;

		// write the result
		m_mapoffset = file_append(&compressed[0], complen + 16);

		// then write the map offset
		uint8_t rawbuf[sizeof(uint64_t)];
		be_write(rawbuf, m_mapoffset, 8);
		file_write(m_mapoffset_offset, rawbuf, sizeof(rawbuf));
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		return err;
	}
}

/**
 * @fn  void chd_file::decompress_v5_map()
 *
 * @brief   -------------------------------------------------
 *            decompress_v5_map - decompress the v5 map
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_DECOMPRESSION_ERROR  Thrown when a chderr decompression error error
 *                                          condition occurs.
 */

void chd_file::decompress_v5_map()
{
	// if no offset, we haven't written it yet
	if (m_mapoffset == 0)
	{
		memset(&m_rawmap[0], 0xff, m_rawmap.size());
		return;
	}

	// read the reader
	uint8_t rawbuf[16];
	file_read(m_mapoffset, rawbuf, sizeof(rawbuf));
	uint32_t const mapbytes = be_read(&rawbuf[0], 4);
	uint64_t const firstoffs = be_read(&rawbuf[4], 6);
	util::crc16_t const mapcrc = be_read(&rawbuf[10], 2);
	uint8_t const lengthbits = rawbuf[12];
	uint8_t const selfbits = rawbuf[13];
	uint8_t const parentbits = rawbuf[14];

	// now read the map
	std::vector<uint8_t> compressed(mapbytes);
	file_read(m_mapoffset + 16, &compressed[0], mapbytes);
	bitstream_in bitbuf(&compressed[0], compressed.size());

	// first decode the compression types
	huffman_decoder<16, 8> decoder;
	huffman_error err = decoder.import_tree_rle(bitbuf);
	if (err != HUFFERR_NONE)
		throw std::error_condition(error::DECOMPRESSION_ERROR);
	uint8_t lastcomp = 0;
	int repcount = 0;
	for (int hunknum = 0; hunknum < m_hunkcount; hunknum++)
	{
		uint8_t *rawmap = &m_rawmap[hunknum * 12];
		if (repcount > 0)
			rawmap[0] = lastcomp, repcount--;
		else
		{
			uint8_t val = decoder.decode_one(bitbuf);
			if (val == COMPRESSION_RLE_SMALL)
				rawmap[0] = lastcomp, repcount = 2 + decoder.decode_one(bitbuf);
			else if (val == COMPRESSION_RLE_LARGE)
				rawmap[0] = lastcomp, repcount = 2 + 16 + (decoder.decode_one(bitbuf) << 4), repcount += decoder.decode_one(bitbuf);
			else
				rawmap[0] = lastcomp = val;
		}
	}

	// then iterate through the hunks and extract the needed data
	uint64_t curoffset = firstoffs;
	uint32_t last_self = 0;
	uint64_t last_parent = 0;
	for (int hunknum = 0; hunknum < m_hunkcount; hunknum++)
	{
		uint8_t *rawmap = &m_rawmap[hunknum * 12];
		uint64_t offset = curoffset;
		uint32_t length = 0;
		uint16_t crc = 0;
		switch (rawmap[0])
		{
			// base types
			case COMPRESSION_TYPE_0:
			case COMPRESSION_TYPE_1:
			case COMPRESSION_TYPE_2:
			case COMPRESSION_TYPE_3:
				curoffset += length = bitbuf.read(lengthbits);
				crc = bitbuf.read(16);
				break;

			case COMPRESSION_NONE:
				curoffset += length = m_hunkbytes;
				crc = bitbuf.read(16);
				break;

			case COMPRESSION_SELF:
				last_self = offset = bitbuf.read(selfbits);
				break;

			case COMPRESSION_PARENT:
				offset = bitbuf.read(parentbits);
				last_parent = offset;
				break;

			// pseudo-types; convert into base types
			case COMPRESSION_SELF_1:
				last_self++;
				[[fallthrough]];
			case COMPRESSION_SELF_0:
				rawmap[0] = COMPRESSION_SELF;
				offset = last_self;
				break;

			case COMPRESSION_PARENT_SELF:
				rawmap[0] = COMPRESSION_PARENT;
				last_parent = offset = (uint64_t(hunknum) * uint64_t(m_hunkbytes)) / m_unitbytes;
				break;

			case COMPRESSION_PARENT_1:
				last_parent += m_hunkbytes / m_unitbytes;
				[[fallthrough]];
			case COMPRESSION_PARENT_0:
				rawmap[0] = COMPRESSION_PARENT;
				offset = last_parent;
				break;
		}
		be_write(&rawmap[1], length, 3);
		be_write(&rawmap[4], offset, 6);
		be_write(&rawmap[10], crc, 2);
	}

	// verify the final CRC
	if (util::crc16_creator::simple(&m_rawmap[0], m_hunkcount * 12) != mapcrc)
		throw std::error_condition(error::DECOMPRESSION_ERROR);
}

/**
 * @fn  std::error_condition chd_file::create_common()
 *
 * @brief   -------------------------------------------------
 *            create_common - command path when creating a new CHD file
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_UNSUPPORTED_VERSION  Thrown when a chderr unsupported version error
 *                                          condition occurs.
 * @exception   CHDERR_INVALID_PARAMETER    Thrown when a chderr invalid parameter error
 *                                          condition occurs.
 * @exception   CHDERR_UNKNOWN_COMPRESSION  Thrown when a chderr unknown compression error
 *                                          condition occurs.
 *
 * @return  The new common.
 */

std::error_condition chd_file::create_common()
{
	// wrap in try for proper error handling
	try
	{
		m_version = HEADER_VERSION;
		m_metaoffset = 0;

		// if we have a parent, it must be V3 or later
		if (m_parent && m_parent->version() < 3)
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// must be an even number of units per hunk
		if (m_hunkbytes % m_unitbytes != 0)
			throw std::error_condition(std::errc::invalid_argument);
		if (m_parent && m_unitbytes != m_parent->unit_bytes())
			throw std::error_condition(std::errc::invalid_argument);

		// verify the compression types
		bool found_zero = false;
		for (auto & elem : m_compression)
		{
			// once we hit an empty slot, all later slots must be empty as well
			if (elem == CHD_CODEC_NONE)
				found_zero = true;
			else if (found_zero)
				throw std::error_condition(std::errc::invalid_argument);
			else if (!chd_codec_list::codec_exists(elem))
				throw std::error_condition(error::UNKNOWN_COMPRESSION);
		}

		// create our V5 header
		uint8_t rawheader[V5_HEADER_SIZE];
		memcpy(&rawheader[0], "MComprHD", 8);
		be_write(&rawheader[8], V5_HEADER_SIZE, 4);
		be_write(&rawheader[12], m_version, 4);
		be_write(&rawheader[16], m_compression[0], 4);
		be_write(&rawheader[20], m_compression[1], 4);
		be_write(&rawheader[24], m_compression[2], 4);
		be_write(&rawheader[28], m_compression[3], 4);
		be_write(&rawheader[32], m_logicalbytes, 8);
		be_write(&rawheader[40], compressed() ? 0 : V5_HEADER_SIZE, 8);
		be_write(&rawheader[48], m_metaoffset, 8);
		be_write(&rawheader[56], m_hunkbytes, 4);
		be_write(&rawheader[60], m_unitbytes, 4);
		be_write_sha1(&rawheader[64], util::sha1_t::null);
		be_write_sha1(&rawheader[84], util::sha1_t::null);
		be_write_sha1(&rawheader[104], (m_parent != nullptr) ? m_parent->sha1() : util::sha1_t::null);

		// write the resulting header
		file_write(0, rawheader, sizeof(rawheader));

		// parse it back out to set up fields appropriately
		util::sha1_t parentsha1;
		parse_v5_header(rawheader, parentsha1);

		// writes are obviously permitted; reads only if uncompressed
		m_allow_writes = true;
		m_allow_reads = !compressed();

		// write out the map (if not compressed)
		if (!compressed())
		{
			uint32_t mapsize = m_mapentrybytes * m_hunkcount;
			uint8_t buffer[4096] = { 0 };
			uint64_t offset = m_mapoffset;
			while (mapsize != 0)
			{
				uint32_t bytes_to_write = (std::min<size_t>)(mapsize, sizeof(buffer));
				file_write(offset, buffer, bytes_to_write);
				offset += bytes_to_write;
				mapsize -= bytes_to_write;
			}
		}

		// finish opening the file
		create_open_common();
	}
	catch (std::error_condition const &err)
	{
		// handle errors by closing ourself
		close();
		return err;
	}
	catch (...)
	{
		close();
		throw;
	}
	return std::error_condition();
}

/**
 * @fn  std::error_condition chd_file::open_common(bool writeable)
 *
 * @brief   -------------------------------------------------
 *            open_common - common path when opening an existing CHD file for input
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_FILE         Thrown when a chderr invalid file error condition
 *                                          occurs.
 * @exception   CHDERR_UNSUPPORTED_VERSION  Thrown when a chderr unsupported version error
 *                                          condition occurs.
 * @exception   CHDERR_FILE_NOT_WRITEABLE   Thrown when a chderr file not writeable error
 *                                          condition occurs.
 * @exception   CHDERR_INVALID_PARENT       Thrown when a chderr invalid parent error condition
 *                                          occurs.
 * @exception   CHDERR_INVALID_PARAMETER    Thrown when a chderr invalid parameter error
 *                                          condition occurs.
 *
 * @param   writeable   true if writeable.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file::open_common(bool writeable)
{
	// wrap in try for proper error handling
	try
	{
		// reads are always permitted
		m_allow_reads = true;

		// read the raw header
		uint8_t rawheader[MAX_HEADER_SIZE];
		file_read(0, rawheader, sizeof(rawheader));

		// verify the signature
		if (memcmp(rawheader, "MComprHD", 8) != 0)
			throw std::error_condition(error::INVALID_FILE);

		m_version = be_read(&rawheader[12], 4);

		// read the header if we support it
		util::sha1_t parentsha1 = util::sha1_t::null;
		switch (m_version)
		{
			case 3:     parse_v3_header(rawheader, parentsha1); break;
			case 4:     parse_v4_header(rawheader, parentsha1); break;
			case 5:     parse_v5_header(rawheader, parentsha1); break;
			default:    throw std::error_condition(error::UNSUPPORTED_VERSION);
		}

		// only allow writes to the most recent version
		if (m_version < HEADER_VERSION)
			m_allow_writes = false;

		if (writeable && !m_allow_writes)
			throw std::error_condition(error::FILE_NOT_WRITEABLE);

		// make sure we have a parent if we need one (and don't if we don't)
		if (parentsha1 != util::sha1_t::null)
		{
			if (!m_parent)
				m_parent_missing = true;
			else if (m_parent->sha1() != parentsha1)
				throw std::error_condition(error::INVALID_PARENT);
		}
		else if (m_parent)
			throw std::error_condition(std::errc::invalid_argument);

		// finish opening the file
		create_open_common();
		return std::error_condition();
	}
	catch (std::error_condition const &err)
	{
		// handle errors by closing ourself
		close();
		return err;
	}
}

/**
 * @fn  void chd_file::create_open_common()
 *
 * @brief   -------------------------------------------------
 *            create_open_common - common code for handling creation and opening of a file
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_UNKNOWN_COMPRESSION  Thrown when a chderr unknown compression error
 *                                          condition occurs.
 */

void chd_file::create_open_common()
{
	// verify the compression types and initialize the codecs
	for (int decompnum = 0; decompnum < std::size(m_compression); decompnum++)
	{
		m_decompressor[decompnum] = chd_codec_list::new_decompressor(m_compression[decompnum], *this);
		if (m_decompressor[decompnum] == nullptr && m_compression[decompnum] != 0)
			throw std::error_condition(error::UNKNOWN_COMPRESSION);
	}

	// read the map; v5+ compressed drives need to read and decompress their map
	m_rawmap.resize(m_hunkcount * m_mapentrybytes);
	if (m_version >= 5 && compressed())
		decompress_v5_map();
	else
		file_read(m_mapoffset, &m_rawmap[0], m_rawmap.size());

	// allocate the temporary compressed buffer and a buffer for caching
	m_compressed.resize(m_hunkbytes);
	m_cache.resize(m_hunkbytes);
}

/**
 * @fn  void chd_file::verify_proper_compression_append(uint32_t hunknum)
 *
 * @brief   -------------------------------------------------
 *            verify_proper_compression_append - verify that the given hunk is a proper candidate
 *            for appending to a compressed CHD
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_NOT_OPEN             Thrown when a chderr not open error condition occurs.
 * @exception   CHDERR_HUNK_OUT_OF_RANGE    Thrown when a chderr hunk out of range error
 *                                          condition occurs.
 * @exception   CHDERR_FILE_NOT_WRITEABLE   Thrown when a chderr file not writeable error
 *                                          condition occurs.
 * @exception   CHDERR_COMPRESSION_ERROR    Thrown when a chderr compression error error
 *                                          condition occurs.
 *
 * @param   hunknum The hunknum.
 */

void chd_file::verify_proper_compression_append(uint32_t hunknum)
{
	// punt if no file
	if (!m_file)
		throw std::error_condition(error::NOT_OPEN);

	// return an error if out of range
	if (hunknum >= m_hunkcount)
		throw std::error_condition(error::HUNK_OUT_OF_RANGE);

	// if not writeable, fail
	if (!m_allow_writes)
		throw std::error_condition(error::FILE_NOT_WRITEABLE);

	// compressed writes only via this interface
	if (!compressed())
		throw std::error_condition(error::FILE_NOT_WRITEABLE);

	// only permitted to write new blocks
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	if (rawmap[0] != 0xff)
		throw std::error_condition(error::COMPRESSION_ERROR);

	// if this isn't the first block, only permitted to write immediately
	// after the previous one
	if (hunknum != 0 && rawmap[-12] == 0xff)
		throw std::error_condition(error::COMPRESSION_ERROR);
}

/**
 * @fn  void chd_file::hunk_write_compressed(uint32_t hunknum, int8_t compression, const uint8_t *compressed, uint32_t complength, crc16_t crc16)
 *
 * @brief   -------------------------------------------------
 *            hunk_write_compressed - write a hunk to a compressed CHD, discovering the best
 *            technique
 *          -------------------------------------------------.
 *
 * @param   hunknum     The hunknum.
 * @param   compression The compression.
 * @param   compressed  The compressed.
 * @param   complength  The complength.
 * @param   crc16       The CRC 16.
 */

void chd_file::hunk_write_compressed(uint32_t hunknum, int8_t compression, const uint8_t *compressed, uint32_t complength, util::crc16_t crc16)
{
	// verify that we are appending properly to a compressed file
	verify_proper_compression_append(hunknum);

	// write the final result
	uint64_t offset = file_append(compressed, complength);

	// update the map entry
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = (compression == -1) ? COMPRESSION_NONE : compression;
	be_write(&rawmap[1], complength, 3);
	be_write(&rawmap[4], offset, 6);
	be_write(&rawmap[10], crc16, 2);
}

/**
 * @fn  void chd_file::hunk_copy_from_self(uint32_t hunknum, uint32_t otherhunk)
 *
 * @brief   -------------------------------------------------
 *            hunk_copy_from_self - mark a hunk as being a copy of another hunk in the same CHD
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_PARAMETER    Thrown when a chderr invalid parameter error
 *                                          condition occurs.
 *
 * @param   hunknum     The hunknum.
 * @param   otherhunk   The otherhunk.
 */

void chd_file::hunk_copy_from_self(uint32_t hunknum, uint32_t otherhunk)
{
	// verify that we are appending properly to a compressed file
	verify_proper_compression_append(hunknum);

	// only permitted to reference prior hunks
	if (otherhunk >= hunknum)
		throw std::error_condition(std::errc::invalid_argument);

	// update the map entry
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = COMPRESSION_SELF;
	be_write(&rawmap[1], 0, 3);
	be_write(&rawmap[4], otherhunk, 6);
	be_write(&rawmap[10], 0, 2);
}

/**
 * @fn  void chd_file::hunk_copy_from_parent(uint32_t hunknum, uint64_t parentunit)
 *
 * @brief   -------------------------------------------------
 *            hunk_copy_from_parent - mark a hunk as being a copy of a hunk from a parent CHD
 *          -------------------------------------------------.
 *
 * @param   hunknum     The hunknum.
 * @param   parentunit  The parentunit.
 */

void chd_file::hunk_copy_from_parent(uint32_t hunknum, uint64_t parentunit)
{
	// verify that we are appending properly to a compressed file
	verify_proper_compression_append(hunknum);

	// update the map entry
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = COMPRESSION_PARENT;
	be_write(&rawmap[1], 0, 3);
	be_write(&rawmap[4], parentunit, 6);
	be_write(&rawmap[10], 0, 2);
}

/**
 * @fn  bool chd_file::metadata_find(chd_metadata_tag metatag, int32_t metaindex, metadata_entry &metaentry, bool resume)
 *
 * @brief   -------------------------------------------------
 *            metadata_find - find a metadata entry
 *          -------------------------------------------------.
 *
 * @param   metatag             The metatag.
 * @param   metaindex           The metaindex.
 * @param [in,out]  metaentry   The metaentry.
 * @param   resume              true to resume.
 *
 * @return  true if it succeeds, false if it fails.
 */

bool chd_file::metadata_find(chd_metadata_tag metatag, int32_t metaindex, metadata_entry &metaentry, bool resume)
{
	// start at the beginning unless we're resuming a previous search
	if (!resume)
	{
		metaentry.offset = m_metaoffset;
		metaentry.prev = 0;
	}
	else
	{
		metaentry.prev = metaentry.offset;
		metaentry.offset = metaentry.next;
	}

	// loop until we run out of options
	while (metaentry.offset != 0)
	{
		// read the raw header
		uint8_t raw_meta_header[METADATA_HEADER_SIZE];
		file_read(metaentry.offset, raw_meta_header, sizeof(raw_meta_header));

		// extract the data
		metaentry.metatag = be_read(&raw_meta_header[0], 4);
		metaentry.flags = raw_meta_header[4];
		metaentry.length = be_read(&raw_meta_header[5], 3);
		metaentry.next = be_read(&raw_meta_header[8], 8);

		// if we got a match, proceed
		if (metatag == CHDMETATAG_WILDCARD || metaentry.metatag == metatag)
			if (metaindex-- == 0)
				return true;

		// no match, fetch the next link
		metaentry.prev = metaentry.offset;
		metaentry.offset = metaentry.next;
	}

	// if we get here, we didn't find it
	return false;
}

/**
 * @fn  void chd_file::metadata_set_previous_next(uint64_t prevoffset, uint64_t nextoffset)
 *
 * @brief   -------------------------------------------------
 *            metadata_set_previous_next - set the 'next' offset of a piece of metadata
 *          -------------------------------------------------.
 *
 * @param   prevoffset  The prevoffset.
 * @param   nextoffset  The nextoffset.
 */

void chd_file::metadata_set_previous_next(uint64_t prevoffset, uint64_t nextoffset)
{
	uint64_t offset = 0;

	// if we were the first entry, make the next entry the first
	if (prevoffset == 0)
	{
		offset = m_metaoffset_offset;
		m_metaoffset = nextoffset;
	}

	// otherwise, update the link in the previous header
	else
		offset = prevoffset + 8;

	// create a big-endian version
	uint8_t rawbuf[sizeof(uint64_t)];
	be_write(rawbuf, nextoffset, 8);

	// write to the header and update our local copy
	file_write(offset, rawbuf, sizeof(rawbuf));
}

/**
 * @fn  void chd_file::metadata_update_hash()
 *
 * @brief   -------------------------------------------------
 *            metadata_update_hash - compute the SHA1 hash of all metadata that requests it
 *          -------------------------------------------------.
 */

void chd_file::metadata_update_hash()
{
	// only works for V4 and above, and only for compressed CHDs
	if (m_version < 4 || !compressed())
		return;

	// compute the new overall hash
	util::sha1_t fullsha1 = compute_overall_sha1(raw_sha1());

	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(&rawbuf[0], fullsha1);

	// write to the header
	file_write(m_sha1_offset, rawbuf, sizeof(rawbuf));
}

/**
 * @fn  int CLIB_DECL chd_file::metadata_hash_compare(const void *elem1, const void *elem2)
 *
 * @brief   -------------------------------------------------
 *            metadata_hash_compare - compare two hash entries
 *          -------------------------------------------------.
 *
 * @param   elem1   The first element.
 * @param   elem2   The second element.
 *
 * @return  A CLIB_DECL.
 */

int CLIB_DECL chd_file::metadata_hash_compare(const void *elem1, const void *elem2)
{
	return memcmp(elem1, elem2, sizeof(metadata_hash));
}



//**************************************************************************
//  CHD COMPRESSOR
//**************************************************************************

/**
 * @fn  chd_file_compressor::chd_file_compressor()
 *
 * @brief   -------------------------------------------------
 *            chd_file_compressor - constructor
 *          -------------------------------------------------.
 */

chd_file_compressor::chd_file_compressor()
	: m_walking_parent(false),
		m_total_in(0),
		m_total_out(0),
		m_read_queue(nullptr),
		m_read_queue_offset(0),
		m_read_done_offset(0),
		m_read_error(false),
		m_work_queue(nullptr),
		m_write_hunk(0)
{
	// zap arrays
	memset(m_codecs, 0, sizeof(m_codecs));

	// allocate work queues
	m_read_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_IO);
	m_work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI);
}

/**
 * @fn  chd_file_compressor::~chd_file_compressor()
 *
 * @brief   -------------------------------------------------
 *            ~chd_file_compressor - destructor
 *          -------------------------------------------------.
 */

chd_file_compressor::~chd_file_compressor()
{
	// free the work queues
	osd_work_queue_free(m_read_queue);
	osd_work_queue_free(m_work_queue);

	// delete allocated arrays
	for (auto & elem : m_codecs)
		delete elem;
}

/**
 * @fn  void chd_file_compressor::compress_begin()
 *
 * @brief   -------------------------------------------------
 *            compress_begin - initiate compression
 *          -------------------------------------------------.
 */

void chd_file_compressor::compress_begin()
{
	// reset state
	m_walking_parent = (m_parent != nullptr);
	m_total_in = 0;
	m_total_out = 0;
	m_compsha1.reset();

	// reset our maps
	m_parent_map.reset();
	m_current_map.reset();

	// reset read state
	m_read_queue_offset = 0;
	m_read_done_offset = 0;
	m_read_error = false;

	// reset work item state
	m_work_buffer.resize(hunk_bytes() * (WORK_BUFFER_HUNKS + 1));
	memset(&m_work_buffer[0], 0, m_work_buffer.size());
	m_compressed_buffer.resize(hunk_bytes() * WORK_BUFFER_HUNKS);
	for (int itemnum = 0; itemnum < WORK_BUFFER_HUNKS; itemnum++)
	{
		work_item &item = m_work_item[itemnum];
		item.m_compressor = this;
		item.m_data = &m_work_buffer[hunk_bytes() * itemnum];
		item.m_compressed = &m_compressed_buffer[hunk_bytes() * itemnum];
		item.m_hash.resize(hunk_bytes() / unit_bytes());
	}

	// initialize codec instances
	for (auto & elem : m_codecs)
	{
		delete elem;
		elem = new chd_compressor_group(*this, m_compression);
	}

	// reset write state
	m_write_hunk = 0;
}

/**
 * @fn  std::error_condition chd_file_compressor::compress_continue(double &progress, double &ratio)
 *
 * @brief   -------------------------------------------------
 *            compress_continue - continue compression
 *          -------------------------------------------------.
 *
 * @param [in,out]  progress    The progress.
 * @param [in,out]  ratio       The ratio.
 *
 * @return  A std::error_condition.
 */

std::error_condition chd_file_compressor::compress_continue(double &progress, double &ratio)
{
	// if we got an error, return an error
	if (m_read_error)
		return std::errc::io_error;

	// if done reading, queue some more
	while (m_read_queue_offset < m_logicalbytes && osd_work_queue_items(m_read_queue) < 2)
	{
		// see if we have enough free work items to read the next half of a buffer
		uint32_t startitem = m_read_queue_offset / hunk_bytes();
		uint32_t enditem = startitem + WORK_BUFFER_HUNKS / 2;
		uint32_t curitem;
		for (curitem = startitem; curitem < enditem; curitem++)
			if (m_work_item[curitem % WORK_BUFFER_HUNKS].m_status != WS_READY)
				break;

		// if it's not all clear, defer
		if (curitem != enditem)
			break;

		// if we're walking the parent, we want one more item to have cleared so we
		// can read an extra hunk there
		if (m_walking_parent && m_work_item[curitem % WORK_BUFFER_HUNKS].m_status != WS_READY)
			break;

		// queue the next read
		for (curitem = startitem; curitem < enditem; curitem++)
			m_work_item[curitem % WORK_BUFFER_HUNKS].m_status = WS_READING;
		osd_work_item_queue(m_read_queue, async_read_static, this, WORK_ITEM_FLAG_AUTO_RELEASE);
		m_read_queue_offset += WORK_BUFFER_HUNKS * hunk_bytes() / 2;
	}

	// flush out any finished items
	while (m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_status == WS_COMPLETE)
	{
		work_item &item = m_work_item[m_write_hunk % WORK_BUFFER_HUNKS];

		// free any OSD work item
		if (item.m_osd != nullptr)
			osd_work_item_release(item.m_osd);
		item.m_osd = nullptr;

		// for parent walking, just add to the hashmap
		if (m_walking_parent)
		{
			uint32_t uph = hunk_bytes() / unit_bytes();
			uint32_t units = uph;
			if (item.m_hunknum == hunk_count() - 1 || !compressed())
				units = 1;
			for (uint32_t unit = 0; unit < units; unit++)
				if (m_parent_map.find(item.m_hash[unit].m_crc16, item.m_hash[unit].m_sha1) == hashmap::NOT_FOUND)
					m_parent_map.add(item.m_hunknum * uph + unit, item.m_hash[unit].m_crc16, item.m_hash[unit].m_sha1);
		}

		// if we're uncompressed, use regular writes
		else if (!compressed())
		{
			std::error_condition err = write_hunk(item.m_hunknum, item.m_data);
			if (err)
				return err;

			// writes of all-0 data don't actually take space, so see if we count this
			chd_codec_type codec = CHD_CODEC_NONE;
			uint32_t complen;
			hunk_info(item.m_hunknum, codec, complen);
			if (codec == CHD_CODEC_NONE)
				m_total_out += m_hunkbytes;
		}

		// for compressing, process the result
		else do
		{
			// first see if the hunk is in the parent or self maps
			uint64_t selfhunk = m_current_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1);
			if (selfhunk != hashmap::NOT_FOUND)
			{
				hunk_copy_from_self(item.m_hunknum, selfhunk);
				break;
			}

			// if not, see if it's in the parent map
			if (m_parent != nullptr)
			{
				uint64_t parentunit = m_parent_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1);
				if (parentunit != hashmap::NOT_FOUND)
				{
					hunk_copy_from_parent(item.m_hunknum, parentunit);
					break;
				}
			}

			// otherwise, append it compressed and add to the self map
			hunk_write_compressed(item.m_hunknum, item.m_compression, item.m_compressed, item.m_complen, item.m_hash[0].m_crc16);
			m_total_out += item.m_complen;
			m_current_map.add(item.m_hunknum, item.m_hash[0].m_crc16, item.m_hash[0].m_sha1);
		} while (false);

		// reset the item and advance
		item.m_status = WS_READY;
		m_write_hunk++;

		// if we hit the end, finalize
		if (m_write_hunk == m_hunkcount)
		{
			// if this is just walking the parent, reset and get ready for compression
			if (m_walking_parent)
			{
				m_walking_parent = false;
				m_read_queue_offset = m_read_done_offset = 0;
				m_write_hunk = 0;
				for (auto & elem : m_work_item)
					elem.m_status = WS_READY;
			}

			// wait for all reads to finish and if we're compressed, write the final SHA1 and map
			else
			{
				osd_work_queue_wait(m_read_queue, 30 * osd_ticks_per_second());
				if (!compressed())
					return std::error_condition();
				set_raw_sha1(m_compsha1.finish());
				return compress_v5_map();
			}
		}
	}

	// update progress and ratio
	if (m_walking_parent)
		progress = double(m_read_done_offset) / double(logical_bytes());
	else
		progress = double(m_write_hunk) / double(m_hunkcount);
	ratio = (m_total_in == 0) ? 1.0 : double(m_total_out) / double(m_total_in);

	// if we're waiting for work, wait
	// sometimes code can get here with .m_status == WS_READY and .m_osd != nullptr, TODO find out why this happens
	while (m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_status != WS_READY &&
		m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_status != WS_COMPLETE &&
		m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_osd != nullptr)
		osd_work_item_wait(m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_osd, osd_ticks_per_second());

	return m_walking_parent ? error::WALKING_PARENT : error::COMPRESSING;
}

/**
 * @fn  void *chd_file_compressor::async_walk_parent_static(void *param, int threadid)
 *
 * @brief   -------------------------------------------------
 *            async_walk_parent - handle asynchronous parent walking operations
 *          -------------------------------------------------.
 *
 * @param [in,out]  param   If non-null, the parameter.
 * @param   threadid        The threadid.
 *
 * @return  null if it fails, else a void*.
 */

void *chd_file_compressor::async_walk_parent_static(void *param, int threadid)
{
	auto *item = reinterpret_cast<work_item *>(param);
	item->m_compressor->async_walk_parent(*item);
	return nullptr;
}

/**
 * @fn  void chd_file_compressor::async_walk_parent(work_item &item)
 *
 * @brief   Asynchronous walk parent.
 *
 * @param [in,out]  item    The item.
 */

void chd_file_compressor::async_walk_parent(work_item &item)
{
	// compute CRC-16 and SHA-1 hashes for each unit, unless we're the last one or we're uncompressed
	uint32_t units = hunk_bytes() / unit_bytes();
	if (item.m_hunknum == m_hunkcount - 1 || !compressed())
		units = 1;
	for (uint32_t unit = 0; unit < units; unit++)
	{
		item.m_hash[unit].m_crc16 = util::crc16_creator::simple(item.m_data + unit * unit_bytes(), hunk_bytes());
		item.m_hash[unit].m_sha1 = util::sha1_creator::simple(item.m_data + unit * unit_bytes(), hunk_bytes());
	}
	item.m_status = WS_COMPLETE;
}

/**
 * @fn  void *chd_file_compressor::async_compress_hunk_static(void *param, int threadid)
 *
 * @brief   -------------------------------------------------
 *            async_compress_hunk - handle asynchronous hunk compression
 *          -------------------------------------------------.
 *
 * @param [in,out]  param   If non-null, the parameter.
 * @param   threadid        The threadid.
 *
 * @return  null if it fails, else a void*.
 */

void *chd_file_compressor::async_compress_hunk_static(void *param, int threadid)
{
	auto *item = reinterpret_cast<work_item *>(param);
	item->m_compressor->async_compress_hunk(*item, threadid);
	return nullptr;
}

/**
 * @fn  void chd_file_compressor::async_compress_hunk(work_item &item, int threadid)
 *
 * @brief   Asynchronous compress hunk.
 *
 * @param [in,out]  item    The item.
 * @param   threadid        The threadid.
 */

void chd_file_compressor::async_compress_hunk(work_item &item, int threadid)
{
	// use our thread's codec
	assert(threadid < std::size(m_codecs));
	item.m_codecs = m_codecs[threadid];

	// compute CRC-16 and SHA-1 hashes
	item.m_hash[0].m_crc16 = util::crc16_creator::simple(item.m_data, hunk_bytes());
	item.m_hash[0].m_sha1 = util::sha1_creator::simple(item.m_data, hunk_bytes());

	// find the best compression scheme, unless we already have a self or parent match
	// (note we may miss a self match from blocks not yet added, but this just results in extra work)
	// TODO: data race
	if (m_current_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1) == hashmap::NOT_FOUND &&
		m_parent_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1) == hashmap::NOT_FOUND)
		item.m_compression = item.m_codecs->find_best_compressor(item.m_data, item.m_compressed, item.m_complen);

	// mark us complete
	item.m_status = WS_COMPLETE;
}

/**
 * @fn  void *chd_file_compressor::async_read_static(void *param, int threadid)
 *
 * @brief   -------------------------------------------------
 *            async_read - handle asynchronous source file reading
 *          -------------------------------------------------.
 *
 * @param [in,out]  param   If non-null, the parameter.
 * @param   threadid        The threadid.
 *
 * @return  null if it fails, else a void*.
 */

void *chd_file_compressor::async_read_static(void *param, int threadid)
{
	reinterpret_cast<chd_file_compressor *>(param)->async_read();
	return nullptr;
}

/**
 * @fn  void chd_file_compressor::async_read()
 *
 * @brief   Asynchronous read.
 */

void chd_file_compressor::async_read()
{
	// if in the error or complete state, stop
	if (m_read_error)
		return;

	// determine parameters for the read
	uint32_t work_buffer_bytes = WORK_BUFFER_HUNKS * hunk_bytes();
	uint32_t numbytes = work_buffer_bytes / 2;
	if (m_read_done_offset + numbytes > logical_bytes())
		numbytes = logical_bytes() - m_read_done_offset;

	// catch any exceptions coming out of here
	try
	{
		// do the read
		uint8_t *dest = &m_work_buffer[0] + (m_read_done_offset % work_buffer_bytes);
		assert(dest == &m_work_buffer[0] || dest == &m_work_buffer[work_buffer_bytes/2]);
		uint64_t end_offset = m_read_done_offset + numbytes;

		// if walking the parent, read in hunks from the parent CHD
		if (m_walking_parent)
		{
			uint8_t *curdest = dest;
			for (uint64_t curoffs = m_read_done_offset; curoffs < end_offset + 1; curoffs += hunk_bytes())
			{
				m_parent->read_hunk(curoffs / hunk_bytes(), curdest);
				curdest += hunk_bytes();
			}
		}

		// otherwise, call the virtual function
		else
			read_data(dest, m_read_done_offset, numbytes);

		// spawn off work for each hunk
		for (uint64_t curoffs = m_read_done_offset; curoffs < end_offset; curoffs += hunk_bytes())
		{
			uint32_t hunknum = curoffs / hunk_bytes();
			work_item &item = m_work_item[hunknum % WORK_BUFFER_HUNKS];
			assert(item.m_status == WS_READING);
			item.m_status = WS_QUEUED;
			item.m_hunknum = hunknum;
			item.m_osd = osd_work_item_queue(m_work_queue, m_walking_parent ? async_walk_parent_static : async_compress_hunk_static, &item, 0);
		}

		// continue the running SHA-1
		if (!m_walking_parent)
		{
			if (compressed())
				m_compsha1.append(dest, numbytes);
			m_total_in += numbytes;
		}

		// advance the read pointer
		m_read_done_offset += numbytes;
	}
	catch (std::error_condition const &err)
	{
		fprintf(stderr, "CHD error occurred: %s\n", err.message().c_str());
		m_read_error = true;
	}
	catch (std::exception const &ex)
	{
		fprintf(stderr, "exception occurred: %s\n", ex.what());
		m_read_error = true;
	}
}



//**************************************************************************
//  CHD COMPRESSOR HASHMAP
//**************************************************************************

/**
 * @fn  chd_file_compressor::hashmap::hashmap()
 *
 * @brief   -------------------------------------------------
 *            hashmap - constructor
 *          -------------------------------------------------.
 */

chd_file_compressor::hashmap::hashmap()
	: m_block_list(new entry_block(nullptr))
{
	// initialize the map to empty
	memset(m_map, 0, sizeof(m_map));
}

/**
 * @fn  chd_file_compressor::hashmap::~hashmap()
 *
 * @brief   -------------------------------------------------
 *            ~hashmap - destructor
 *          -------------------------------------------------.
 */

chd_file_compressor::hashmap::~hashmap()
{
	reset();
	delete m_block_list;
}

/**
 * @fn  void chd_file_compressor::hashmap::reset()
 *
 * @brief   -------------------------------------------------
 *            reset - reset the state of the map
 *          -------------------------------------------------.
 */

void chd_file_compressor::hashmap::reset()
{
	// delete all the blocks
	while (m_block_list->m_next != nullptr)
	{
		entry_block *block = m_block_list;
		m_block_list = block->m_next;
		delete block;
	}
	m_block_list->m_nextalloc = 0;

	// reset the hash
	memset(m_map, 0, sizeof(m_map));
}

/**
 * @fn  uint64_t chd_file_compressor::hashmap::find(crc16_t crc16, sha1_t sha1)
 *
 * @brief   -------------------------------------------------
 *            find - find an item in the CRC map
 *          -------------------------------------------------.
 *
 * @param   crc16   The CRC 16.
 * @param   sha1    The first sha.
 *
 * @return  An uint64_t.
 */

uint64_t chd_file_compressor::hashmap::find(util::crc16_t crc16, util::sha1_t sha1)
{
	// look up the entry in the map
	for (entry_t *entry = m_map[crc16]; entry != nullptr; entry = entry->m_next)
		if (entry->m_sha1 == sha1)
			return entry->m_itemnum;
	return NOT_FOUND;
}

/**
 * @fn  void chd_file_compressor::hashmap::add(uint64_t itemnum, crc16_t crc16, sha1_t sha1)
 *
 * @brief   -------------------------------------------------
 *            add - add an item to the CRC map
 *          -------------------------------------------------.
 *
 * @param   itemnum The itemnum.
 * @param   crc16   The CRC 16.
 * @param   sha1    The first sha.
 */

void chd_file_compressor::hashmap::add(uint64_t itemnum, util::crc16_t crc16, util::sha1_t sha1)
{
	// add to the appropriate map
	if (m_block_list->m_nextalloc == std::size(m_block_list->m_array))
		m_block_list = new entry_block(m_block_list);
	entry_t *entry = &m_block_list->m_array[m_block_list->m_nextalloc++];
	entry->m_itemnum = itemnum;
	entry->m_sha1 = sha1;
	entry->m_next = m_map[crc16];
	m_map[crc16] = entry;
}
