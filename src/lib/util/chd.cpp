// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    MAME Compressed Hunks of Data file format

***************************************************************************/

#include "chd.h"

#include "avhuff.h"
#include "cdrom.h"
#include "corefile.h"
#include "coretmpl.h"
#include "flac.h"
#include "hashing.h"
#include "multibyte.h"

#include "eminline.h"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <new>
#include <tuple>


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
	uint64_t                offset;         // offset within the file of the header
	uint64_t                next;           // offset within the file of the next header
	uint64_t                prev;           // offset within the file of the previous header
	uint32_t                length;         // length of the metadata
	uint32_t                metatag;        // metadata tag
	uint8_t                 flags;          // flag bits
};


// ======================> metadata_hash

struct chd_file::metadata_hash
{
	uint8_t                 tag[4];         // tag of the metadata in big-endian
	util::sha1_t            sha1;           // hash data
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  be_read_sha1 - fetch a sha1_t from a data
//  stream in bigendian order
//-------------------------------------------------

inline util::sha1_t chd_file::be_read_sha1(const uint8_t *base) const noexcept
{
	util::sha1_t result;
	memcpy(&result.m_raw[0], base, sizeof(result.m_raw));
	return result;
}


//-------------------------------------------------
//  be_write_sha1 - write a sha1_t to a data
//  stream in bigendian order
//-------------------------------------------------

inline void chd_file::be_write_sha1(uint8_t *base, util::sha1_t value) noexcept
{
	memcpy(base, &value.m_raw[0], sizeof(value.m_raw));
}


//-------------------------------------------------
//  file_read - read from the file at the given
//  offset.
//-------------------------------------------------

inline std::error_condition chd_file::file_read(uint64_t offset, void *dest, uint32_t length) const noexcept
{
	// no file = failure
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// seek and read
	std::error_condition err;
	err = m_file->seek(offset, SEEK_SET);
	if (UNEXPECTED(err))
		return err;
	size_t count;
	std::tie(err, count) = read(*m_file, dest, length);
	if (UNEXPECTED(!err && (count != length)))
		return std::error_condition(std::errc::io_error); // TODO: revisit this error code (happens if file is truncated)
	return err;
}


//-------------------------------------------------
//  file_write - write to the file at the given
//  offset.
//-------------------------------------------------

inline std::error_condition chd_file::file_write(uint64_t offset, const void *source, uint32_t length) noexcept
{
	// no file = failure
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// seek and write
	std::error_condition err;
	err = m_file->seek(offset, SEEK_SET);
	if (UNEXPECTED(err))
		return err;
	return write(*m_file, source, length).first;
}


//-------------------------------------------------
//  file_append - append to the file at the given
//  offset, ensuring we start at the given
//  alignment; on failure throw an error
//-------------------------------------------------

inline uint64_t chd_file::file_append(const void *source, uint32_t length, uint32_t alignment)
{
	// no file = failure
	if (UNEXPECTED(!m_file))
		throw std::error_condition(error::NOT_OPEN);

	// seek to the end and align if necessary
	std::error_condition err;
	err = m_file->seek(0, SEEK_END);
	if (UNEXPECTED(err))
		throw err;
	if (alignment != 0)
	{
		uint64_t offset;
		err = m_file->tell(offset);
		if (UNEXPECTED(err))
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
				std::tie(err, count) = write(*m_file, buffer, bytes_to_write);
				if (UNEXPECTED(err))
					throw err;
				delta -= count;
			}
		}
	}

	// write the real data
	uint64_t offset;
	err = m_file->tell(offset);
	if (UNEXPECTED(err))
		throw err;
	std::tie(err, std::ignore) = write(*m_file, source, length);
	if (UNEXPECTED(err))
		throw err;
	return offset;
}


//-------------------------------------------------
//  bits_for_value - return the number of bits
//  necessary to represent all numbers 0..value
//-------------------------------------------------

inline uint8_t chd_file::bits_for_value(uint64_t value) noexcept
{
	uint8_t result = 0;
	while (value != 0)
	{
		value >>= 1;
		result++;
	}
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

bool chd_file::parent_missing() const noexcept
{
	if (m_parent_missing)
		return true;
	else if (!m_parent)
		return false;
	else
		return m_parent->parent_missing();
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

util::sha1_t chd_file::sha1() const noexcept
{
	// read the big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	std::error_condition err = file_read(m_sha1_offset, rawbuf, sizeof(rawbuf));
	if (UNEXPECTED(err))
		return util::sha1_t::null; // on failure, return null
	return be_read_sha1(rawbuf);
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

util::sha1_t chd_file::raw_sha1() const noexcept
{
	try
	{
		// determine offset within the file for data-only
		if (UNEXPECTED(!m_rawsha1_offset))
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// read the big-endian version
		uint8_t rawbuf[sizeof(util::sha1_t)];
		std::error_condition err = file_read(m_rawsha1_offset, rawbuf, sizeof(rawbuf));
		if (UNEXPECTED(err))
			throw err;
		return be_read_sha1(rawbuf);
	}
	catch (std::error_condition const &)
	{
		// on failure, return null
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

util::sha1_t chd_file::parent_sha1() const noexcept
{
	try
	{
		// determine offset within the file
		if (UNEXPECTED(!m_parentsha1_offset))
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// read the big-endian version
		uint8_t rawbuf[sizeof(util::sha1_t)];
		std::error_condition err = file_read(m_parentsha1_offset, rawbuf, sizeof(rawbuf));
		if (UNEXPECTED(err))
			throw err;
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
	switch (m_version)
	{
	// v3/v4 map entries
	case 3:
	case 4:
		{
			uint8_t const *const rawmap = &m_rawmap[16 * hunknum];
			switch (rawmap[15] & V34_MAP_ENTRY_FLAG_TYPE_MASK)
			{
			case V34_MAP_ENTRY_TYPE_COMPRESSED:
				compressor = CHD_CODEC_ZLIB;
				compbytes = get_u16be(&rawmap[12]) + (rawmap[14] << 16);
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
		}
		break;

	// v5 map entries
	case 5:
		{
			uint8_t const *const rawmap = &m_rawmap[m_mapentrybytes * hunknum];

			if (!compressed())
			{
				// uncompressed case
				if (get_u32be(&rawmap[0]) == 0)
				{
					compressor = CHD_CODEC_PARENT;
					compbytes = 0;
				}
				else
				{
					compressor = CHD_CODEC_NONE;
					compbytes = m_hunkbytes;
				}
			}
			else
			{
				// compressed case
				switch (rawmap[0])
				{
				case COMPRESSION_TYPE_0:
				case COMPRESSION_TYPE_1:
				case COMPRESSION_TYPE_2:
				case COMPRESSION_TYPE_3:
					compressor = m_compression[rawmap[0]];
					compbytes = get_u24be(&rawmap[1]);
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
			}
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

std::error_condition chd_file::set_raw_sha1(util::sha1_t rawdata) noexcept
{
	uint64_t const offset = (m_rawsha1_offset != 0) ? m_rawsha1_offset : m_sha1_offset;
	assert(offset != 0);

	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(rawbuf, rawdata);

	// write to the header
	std::error_condition err = file_write(offset, rawbuf, sizeof(rawbuf));
	if (UNEXPECTED(err))
		return err;

	try
	{
		// if we have a separate rawsha1_offset, update the full sha1 as well
		if (m_rawsha1_offset != 0)
			metadata_update_hash();
	}
	catch (std::error_condition const &err)
	{
		return err;
	}
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
	}

	return std::error_condition();
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

std::error_condition chd_file::set_parent_sha1(util::sha1_t parent) noexcept
{
	// if no file, fail
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::INVALID_FILE);

	assert(m_parentsha1_offset != 0);

	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(rawbuf, parent);

	// write to the header
	return file_write(m_parentsha1_offset, rawbuf, sizeof(rawbuf));
}

/**
 * @fn  std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, const chd_codec_type (&compression)[4])
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

std::error_condition chd_file::create(
		util::random_read_write::ptr &&file,
		uint64_t logicalbytes,
		uint32_t hunkbytes,
		uint32_t unitbytes,
		const chd_codec_type (&compression)[4])
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;
	else if (UNEXPECTED(!file))
		return std::errc::invalid_argument;

	// set the header parameters
	m_logicalbytes = logicalbytes;
	m_hunkbytes = hunkbytes;
	m_unitbytes = unitbytes;
	memcpy(m_compression, compression, sizeof(m_compression));
	m_parent.reset();

	// take ownership of the file
	m_file = std::move(file);
	return create_common();
}

/**
 * @fn  std::error_condition chd_file::create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, const chd_codec_type (&compression)[4], chd_file &parent)
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

std::error_condition chd_file::create(
		util::random_read_write::ptr &&file,
		uint64_t logicalbytes,
		uint32_t hunkbytes,
		const chd_codec_type (&compression)[4],
		chd_file &parent)
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;
	else if (UNEXPECTED(!file))
		return std::errc::invalid_argument;

	// set the header parameters
	m_logicalbytes = logicalbytes;
	m_hunkbytes = hunkbytes;
	m_unitbytes = parent.unit_bytes();
	memcpy(m_compression, compression, sizeof(m_compression));
	m_parent = std::shared_ptr<chd_file>(std::shared_ptr<chd_file>(), &parent);

	// take ownership of the file
	m_file = std::move(file);
	return create_common();
}

/**
 * @fn  std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, const chd_codec_type (&compression)[4])
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

std::error_condition chd_file::create(
		std::string_view filename,
		uint64_t logicalbytes,
		uint32_t hunkbytes,
		uint32_t unitbytes,
		const chd_codec_type (&compression)[4])
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;

	// create the new file
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
	if (UNEXPECTED(filerr))
		return filerr;

	// create the file normally, then claim the file
	std::error_condition chderr = create(std::move(file), logicalbytes, hunkbytes, unitbytes, compression);

	// if an error happened, close and delete the file
	if (UNEXPECTED(chderr))
	{
		file.reset();
		osd_file::remove(std::string(filename)); // FIXME: allow osd_file to use std::string_view
	}
	return chderr;
}

/**
 * @fn  std::error_condition chd_file::create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, const chd_codec_type (&compression)[4], chd_file &parent)
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

std::error_condition chd_file::create(
		std::string_view filename,
		uint64_t logicalbytes,
		uint32_t hunkbytes,
		const chd_codec_type (&compression)[4],
		chd_file &parent)
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;

	// create the new file
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
	if (UNEXPECTED(filerr))
		return filerr;

	// create the file normally, then claim the file
	std::error_condition chderr = create(std::move(file), logicalbytes, hunkbytes, compression, parent);

	// if an error happened, close and delete the file
	if (UNEXPECTED(chderr))
	{
		file.reset();
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

std::error_condition chd_file::open(
		std::string_view filename,
		bool writeable,
		chd_file *parent,
		const open_parent_func &open_parent)
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;

	// open the file
	const uint32_t openflags = writeable ? (OPEN_FLAG_READ | OPEN_FLAG_WRITE) : OPEN_FLAG_READ;
	util::core_file::ptr file;
	std::error_condition filerr = util::core_file::open(filename, openflags, file);
	if (UNEXPECTED(filerr))
		return filerr;

	// now open the CHD
	return open(std::move(file), writeable, parent, open_parent);
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

std::error_condition chd_file::open(
		util::random_read_write::ptr &&file,
		bool writeable,
		chd_file *parent,
		const open_parent_func &open_parent)
{
	// make sure we don't already have a file open
	if (UNEXPECTED(m_file))
		return error::ALREADY_OPEN;
	else if (UNEXPECTED(!file))
		return std::errc::invalid_argument;

	// open the file
	m_file = std::move(file);
	m_parent = std::shared_ptr<chd_file>(std::shared_ptr<chd_file>(), parent);
	m_cachehunk = ~0;
	return open_common(writeable, open_parent);
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
	std::fill(std::begin(m_compression), std::end(m_compression), 0);
	m_parent.reset();
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

std::error_condition chd_file::codec_process_hunk(uint32_t hunknum)
{
	// punt if no file
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// return an error if out of range
	if (UNEXPECTED(hunknum >= m_hunkcount))
		return std::error_condition(error::HUNK_OUT_OF_RANGE);

	// wrap this for clean reporting
	try
	{
		// get a pointer to the map entry
		switch (m_version)
		{
		// v3/v4 map entries
		case 3:
		case 4:
			{
				uint8_t const *const rawmap = &m_rawmap[16 * hunknum];
				uint64_t const blockoffs = get_u64be(&rawmap[0]);
				switch (rawmap[15] & V34_MAP_ENTRY_FLAG_TYPE_MASK)
				{
				case V34_MAP_ENTRY_TYPE_COMPRESSED:
					{
						uint32_t const blocklen = get_u16be(&rawmap[12]) | (uint32_t(rawmap[14]) << 16);
						std::error_condition err = file_read(blockoffs, &m_compressed[0], blocklen);
						if (UNEXPECTED(err))
							return err;
						m_decompressor[0]->process(&m_compressed[0], blocklen);
						return std::error_condition();
					}

				case V34_MAP_ENTRY_TYPE_UNCOMPRESSED:
				case V34_MAP_ENTRY_TYPE_MINI:
					return std::error_condition(error::UNSUPPORTED_FORMAT);

				case V34_MAP_ENTRY_TYPE_SELF_HUNK:
					return codec_process_hunk(blockoffs);

				case V34_MAP_ENTRY_TYPE_PARENT_HUNK:
					if (UNEXPECTED(m_parent_missing))
						return std::error_condition(error::REQUIRES_PARENT);
					return m_parent->codec_process_hunk(blockoffs);
				}
			}
			break;

		// v5 map entries
		case 5:
			{
				if (UNEXPECTED(!compressed()))
					return std::error_condition(error::UNSUPPORTED_FORMAT);

				// compressed case
				uint8_t const *const rawmap = &m_rawmap[m_mapentrybytes * hunknum];
				uint32_t const blocklen = get_u24be(&rawmap[1]);
				uint64_t const blockoffs = get_u48be(&rawmap[4]);
				switch (rawmap[0])
				{
				case COMPRESSION_TYPE_0:
				case COMPRESSION_TYPE_1:
				case COMPRESSION_TYPE_2:
				case COMPRESSION_TYPE_3:
					{
						std::error_condition err = file_read(blockoffs, &m_compressed[0], blocklen);
						if (UNEXPECTED(err))
							return err;
						auto &decompressor = *m_decompressor[rawmap[0]];
						decompressor.process(&m_compressed[0], blocklen);
						return std::error_condition();
					}

				case COMPRESSION_NONE:
					return std::error_condition(error::UNSUPPORTED_FORMAT);

				case COMPRESSION_SELF:
					return codec_process_hunk(blockoffs);

				case COMPRESSION_PARENT:
					if (UNEXPECTED(m_parent_missing))
						return std::error_condition(error::REQUIRES_PARENT);
					return m_parent->codec_process_hunk(blockoffs / (m_parent->hunk_bytes() / m_parent->unit_bytes()));
				}
				break;
			}
		}

		// if we get here, the map contained an unsupported block type
		return std::error_condition(error::INVALID_DATA);
	}
	catch (std::error_condition const &err)
	{
		// just return errors
		return err;
	}
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
 * @return  An error condition.
 */

std::error_condition chd_file::read_hunk(uint32_t hunknum, void *buffer)
{
	// punt if no file
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// return an error if out of range
	if (UNEXPECTED(hunknum >= m_hunkcount))
		return std::error_condition(error::HUNK_OUT_OF_RANGE);

	auto *const dest = reinterpret_cast<uint8_t *>(buffer);

	// wrap this for clean reporting
	try
	{
		// get a pointer to the map entry
		switch (m_version)
		{
		// v3/v4 map entries
		case 3:
		case 4:
			{
				uint8_t const *const rawmap = &m_rawmap[16 * hunknum];
				uint64_t const blockoffs = get_u64be(&rawmap[0]);
				util::crc32_t const blockcrc = get_u32be(&rawmap[8]);
				bool const nocrc = rawmap[15] & V34_MAP_ENTRY_FLAG_NO_CRC;
				switch (rawmap[15] & V34_MAP_ENTRY_FLAG_TYPE_MASK)
				{
				case V34_MAP_ENTRY_TYPE_COMPRESSED:
					{
						uint32_t const blocklen = get_u16be(&rawmap[12]) | (uint32_t(rawmap[14]) << 16);
						std::error_condition err = file_read(blockoffs, &m_compressed[0], blocklen);
						if (UNEXPECTED(err))
							return err;
						m_decompressor[0]->decompress(&m_compressed[0], blocklen, dest, m_hunkbytes);
						if (UNEXPECTED(!nocrc && (util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)))
							return std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();
					}

				case V34_MAP_ENTRY_TYPE_UNCOMPRESSED:
					{
						std::error_condition err = file_read(blockoffs, dest, m_hunkbytes);
						if (UNEXPECTED(err))
							return err;
						if (UNEXPECTED(!nocrc && (util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)))
							return std::error_condition(error::DECOMPRESSION_ERROR);
						return std::error_condition();
					}

				case V34_MAP_ENTRY_TYPE_MINI:
					put_u64be(dest, blockoffs);
					for (uint32_t bytes = 8; bytes < m_hunkbytes; bytes++)
						dest[bytes] = dest[bytes - 8];
					if (UNEXPECTED(!nocrc && (util::crc32_creator::simple(dest, m_hunkbytes) != blockcrc)))
						return std::error_condition(error::DECOMPRESSION_ERROR);
					return std::error_condition();

				case V34_MAP_ENTRY_TYPE_SELF_HUNK:
					return read_hunk(blockoffs, dest);

				case V34_MAP_ENTRY_TYPE_PARENT_HUNK:
					if (UNEXPECTED(m_parent_missing))
						return std::error_condition(error::REQUIRES_PARENT);
					return m_parent->read_hunk(blockoffs, dest);
				}
			}
			break;

		// v5 map entries
		case 5:
			{
				uint8_t const *const rawmap = &m_rawmap[m_mapentrybytes * hunknum];

				if (!compressed())
				{
					// uncompressed case
					uint64_t const blockoffs = mulu_32x32(get_u32be(rawmap), m_hunkbytes);
					if (blockoffs != 0)
						return file_read(blockoffs, dest, m_hunkbytes);
					else if (UNEXPECTED(m_parent_missing))
						return std::error_condition(error::REQUIRES_PARENT);
					else if (m_parent)
						return m_parent->read_hunk(hunknum, dest);
					else
						memset(dest, 0, m_hunkbytes);
					return std::error_condition();
				}
				else
				{
					// compressed case
					uint32_t const blocklen = get_u24be(&rawmap[1]);
					uint64_t const blockoffs = get_u48be(&rawmap[4]);
					util::crc16_t const blockcrc = get_u16be(&rawmap[10]);
					switch (rawmap[0])
					{
					case COMPRESSION_TYPE_0:
					case COMPRESSION_TYPE_1:
					case COMPRESSION_TYPE_2:
					case COMPRESSION_TYPE_3:
						{
							std::error_condition err = file_read(blockoffs, &m_compressed[0], blocklen);
							if (UNEXPECTED(err))
								return err;
							auto &decompressor = *m_decompressor[rawmap[0]];
							decompressor.decompress(&m_compressed[0], blocklen, dest, m_hunkbytes);
							util::crc16_t const calculated = !decompressor.lossy()
									? util::crc16_creator::simple(dest, m_hunkbytes)
									: util::crc16_creator::simple(&m_compressed[0], blocklen);
							if (UNEXPECTED(calculated != blockcrc))
								return std::error_condition(error::DECOMPRESSION_ERROR);
							return std::error_condition();
						}

					case COMPRESSION_NONE:
						{
							std::error_condition err = file_read(blockoffs, dest, m_hunkbytes);
							if (UNEXPECTED(err))
								return err;
							if (UNEXPECTED(util::crc16_creator::simple(dest, m_hunkbytes) != blockcrc))
								return std::error_condition(error::DECOMPRESSION_ERROR);
							return std::error_condition();
						}

					case COMPRESSION_SELF:
						return read_hunk(blockoffs, dest);

					case COMPRESSION_PARENT:
						if (UNEXPECTED(m_parent_missing))
							return std::error_condition(error::REQUIRES_PARENT);
						return m_parent->read_bytes(blockoffs * m_parent->unit_bytes(), dest, m_hunkbytes);
					}
				}
				break;
			}
		}

		// if we get here, the map contained an unsupported block type
		return std::error_condition(error::INVALID_DATA);
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
	// punt if no file
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// return an error if out of range
	if (UNEXPECTED(hunknum >= m_hunkcount))
		return std::error_condition(error::HUNK_OUT_OF_RANGE);

	// if not writeable, fail
	if (UNEXPECTED(!m_allow_writes))
		return std::error_condition(error::FILE_NOT_WRITEABLE);

	// uncompressed writes only via this interface
	if (UNEXPECTED(compressed()))
		return std::error_condition(error::FILE_NOT_WRITEABLE);

	// see if we have allocated the space on disk for this hunk
	uint8_t *const rawmap = &m_rawmap[hunknum * 4];
	uint32_t rawentry = get_u32be(rawmap);

	// if not, allocate one now
	if (rawentry == 0)
	{
		// first make sure we need to allocate it
		bool all_zeros = true;
		const auto *scan = reinterpret_cast<const uint32_t *>(buffer);
		for (uint32_t index = 0; index < m_hunkbytes / 4; index++)
		{
			if (scan[index] != 0)
			{
				all_zeros = false;
				break;
			}
		}

		// if it's all zeros, do nothing more
		if (all_zeros)
			return std::error_condition();

		// wrap this for clean reporting
		try
		{
			// append new data to the end of the file, aligning the first chunk
			rawentry = file_append(buffer, m_hunkbytes, m_hunkbytes) / m_hunkbytes;
		}
		catch (std::error_condition const &err)
		{
			// just return errors
			return err;
		}

		// write the map entry back
		put_u32be(rawmap, rawentry);
		std::error_condition err = file_write(m_mapoffset + hunknum * 4, rawmap, 4);
		if (UNEXPECTED(err))
			return err;

		// update the cached hunk if we just wrote it
		if (hunknum == m_cachehunk && buffer != &m_cache[0])
			memcpy(&m_cache[0], buffer, m_hunkbytes);

		return std::error_condition();
	}
	else
	{
		// otherwise, just overwrite
		return file_write(uint64_t(rawentry) * uint64_t(m_hunkbytes), buffer, m_hunkbytes);
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
	uint32_t const first_hunk = offset / m_hunkbytes;
	uint32_t const last_hunk = (offset + bytes - 1) / m_hunkbytes;
	auto *dest = reinterpret_cast<uint8_t *>(buffer);
	for (uint32_t curhunk = first_hunk; curhunk <= last_hunk; curhunk++)
	{
		// determine start/end boundaries
		uint32_t const startoffs = (curhunk == first_hunk) ? (offset % m_hunkbytes) : 0;
		uint32_t const endoffs = (curhunk == last_hunk) ? ((offset + bytes - 1) % m_hunkbytes) : (m_hunkbytes - 1);

		if ((startoffs == 0) && (endoffs == m_hunkbytes - 1) && (curhunk != m_cachehunk))
		{
			// if it's a full block, just read directly from disk unless it's the cached hunk
			std::error_condition err = read_hunk(curhunk, dest);
			if (UNEXPECTED(err))
				return err;
		}
		else
		{
			// otherwise, read from the cache
			if (curhunk != m_cachehunk)
			{
				std::error_condition err = read_hunk(curhunk, &m_cache[0]);
				if (UNEXPECTED(err))
					return err;
				m_cachehunk = curhunk;
			}
			memcpy(dest, &m_cache[startoffs], endoffs + 1 - startoffs);
		}

		// advance
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
	uint32_t const first_hunk = offset / m_hunkbytes;
	uint32_t const last_hunk = (offset + bytes - 1) / m_hunkbytes;
	auto const *source = reinterpret_cast<uint8_t const *>(buffer);
	for (uint32_t curhunk = first_hunk; curhunk <= last_hunk; curhunk++)
	{
		// determine start/end boundaries
		uint32_t const startoffs = (curhunk == first_hunk) ? (offset % m_hunkbytes) : 0;
		uint32_t const endoffs = (curhunk == last_hunk) ? ((offset + bytes - 1) % m_hunkbytes) : (m_hunkbytes - 1);

		std::error_condition err;
		if ((startoffs == 0) && (endoffs == m_hunkbytes - 1) && (curhunk != m_cachehunk))
		{
			// if it's a full block, just write directly to disk unless it's the cached hunk
			err = write_hunk(curhunk, source);
		}
		else
		{
			// otherwise, write from the cache
			if (curhunk != m_cachehunk)
			{
				err = read_hunk(curhunk, &m_cache[0]);
				if (UNEXPECTED(err))
					return err;
				m_cachehunk = curhunk;
			}
			memcpy(&m_cache[startoffs], source, endoffs + 1 - startoffs);
			err = write_hunk(curhunk, &m_cache[0]);
		}

		// handle errors and advance
		if (UNEXPECTED(err))
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
 * @return  An error condition.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::string &output)
{
	// if we didn't find it, just return
	metadata_entry metaentry;
	if (std::error_condition err = metadata_find(searchtag, searchindex, metaentry))
		return err;

	// read the metadata
	try { output.assign(metaentry.length, '\0'); }
	catch (std::bad_alloc const &) { return std::errc::not_enough_memory; }
	return file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
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
 * @return  An error condition.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output)
{
	// if we didn't find it, just return
	metadata_entry metaentry;
	if (std::error_condition err = metadata_find(searchtag, searchindex, metaentry))
		return err;

	// read the metadata
	try { output.resize(metaentry.length); }
	catch (std::bad_alloc const &) { return std::errc::not_enough_memory; }
	return file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
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
 * @return  An error condition.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, void *output, uint32_t outputlen, uint32_t &resultlen)
{
	// if we didn't find it, just return
	metadata_entry metaentry;
	if (std::error_condition err = metadata_find(searchtag, searchindex, metaentry))
		return err;

	// read the metadata
	resultlen = metaentry.length;
	return file_read(metaentry.offset + METADATA_HEADER_SIZE, output, std::min(outputlen, resultlen));
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
 * @return  An error condition.
 */

std::error_condition chd_file::read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output, chd_metadata_tag &resulttag, uint8_t &resultflags)
{
	std::error_condition err;

	// if we didn't find it, just return
	metadata_entry metaentry;
	err = metadata_find(searchtag, searchindex, metaentry);
	if (err)
		return err;

	// read the metadata
	try { output.resize(metaentry.length); }
	catch (std::bad_alloc const &) { return std::errc::not_enough_memory; }
	err = file_read(metaentry.offset + METADATA_HEADER_SIZE, &output[0], metaentry.length);
	if (UNEXPECTED(err))
		return err;
	resulttag = metaentry.metatag;
	resultflags = metaentry.flags;
	return std::error_condition();
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
	// must write at least 1 byte and no more than 16MB
	if (UNEXPECTED((inputlen < 1) || (inputlen >= 16 * 1024 * 1024)))
		return std::error_condition(std::errc::invalid_argument);

	// find the entry if it already exists
	metadata_entry metaentry;
	bool finished = false;
	std::error_condition err = metadata_find(metatag, metaindex, metaentry);
	if (!err)
	{
		if (inputlen <= metaentry.length)
		{
			// if the new data fits over the old data, just overwrite
			err = file_write(metaentry.offset + METADATA_HEADER_SIZE, inputbuf, inputlen);
			if (UNEXPECTED(err))
				return err;

			// if the lengths don't match, we need to update the length in our header
			if (inputlen != metaentry.length)
			{
				uint8_t length[3];
				put_u24be(length, inputlen);
				err = file_write(metaentry.offset + 5, length, sizeof(length));
				if (UNEXPECTED(err))
					return err;
			}

			// indicate we did everything
			finished = true;
		}
		else
		{
			// if it doesn't fit, unlink the current entry
			err = metadata_set_previous_next(metaentry.prev, metaentry.next);
			if (UNEXPECTED(err))
				return err;
		}
	}
	else if (UNEXPECTED(err != error::METADATA_NOT_FOUND))
	{
		return err;
	}

	// wrap this for clean reporting
	try
	{
		// if not yet done, create a new entry and append
		if (!finished)
		{
			// now build us a new entry
			uint8_t raw_meta_header[METADATA_HEADER_SIZE];
			put_u32be(&raw_meta_header[0], metatag);
			raw_meta_header[4] = flags;
			put_u24be(&raw_meta_header[5], inputlen & 0x00ffffff);
			put_u64be(&raw_meta_header[8], 0);

			// append the new header, then the data
			uint64_t offset = file_append(raw_meta_header, sizeof(raw_meta_header));
			file_append(inputbuf, inputlen);

			// set the previous entry to point to us
			err = metadata_set_previous_next(metaentry.prev, offset);
			if (UNEXPECTED(err))
				return err;
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
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
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
	// find the entry
	metadata_entry metaentry;
	if (std::error_condition err = metadata_find(metatag, metaindex, metaentry))
		return err;

	// point the previous to the next, unlinking us
	return metadata_set_previous_next(metaentry.prev, metaentry.next);
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
	// iterate over metadata entries in the source
	std::vector<uint8_t> filedata;
	metadata_entry metaentry;
	metaentry.metatag = 0;
	metaentry.length = 0;
	metaentry.next = 0;
	metaentry.flags = 0;
	std::error_condition err;
	for (err = source.metadata_find(CHDMETATAG_WILDCARD, 0, metaentry); !err; err = source.metadata_find(CHDMETATAG_WILDCARD, 0, metaentry, true))
	{
		// read the metadata item
		try { filedata.resize(metaentry.length); }
		catch (std::bad_alloc const &) { return std::errc::not_enough_memory; }
		err = source.file_read(metaentry.offset + METADATA_HEADER_SIZE, &filedata[0], metaentry.length);
		if (UNEXPECTED(err))
			return err;

		// write it to the destination
		err = write_metadata(metaentry.metatag, (uint32_t)-1, &filedata[0], metaentry.length, metaentry.flags);
		if (UNEXPECTED(err))
			return err;
	}
	if (err == error::METADATA_NOT_FOUND)
		return std::error_condition();
	else
		return err;
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
	std::error_condition err;
	for (err = metadata_find(CHDMETATAG_WILDCARD, 0, metaentry); !err; err = metadata_find(CHDMETATAG_WILDCARD, 0, metaentry, true))
	{
		// if not checksumming, continue
		if (!(metaentry.flags & CHD_MDFLAGS_CHECKSUM))
			continue;

		// allocate memory and read the data
		filedata.resize(metaentry.length);
		err = file_read(metaentry.offset + METADATA_HEADER_SIZE, &filedata[0], metaentry.length);
		if (UNEXPECTED(err))
			throw err;

		// create an entry for this metadata and add it
		metadata_hash hashentry;
		put_u32be(hashentry.tag, metaentry.metatag);
		hashentry.sha1 = util::sha1_creator::simple(&filedata[0], metaentry.length);
		hasharray.push_back(hashentry);
	}
	if (err != error::METADATA_NOT_FOUND)
		throw err;

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
		return cdrom_file::FRAME_SIZE;

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
	if (UNEXPECTED(get_u32be(&rawheader[8]) != V3_HEADER_SIZE))
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = get_u64be(&rawheader[28]);
	m_mapoffset = 120;
	m_metaoffset = get_u64be(&rawheader[36]);
	m_hunkbytes = get_u32be(&rawheader[76]);
	m_hunkcount = get_u32be(&rawheader[24]);

	// extract parent SHA-1
	uint32_t flags = get_u32be(&rawheader[16]);
	m_allow_writes = (flags & 2) == 0;

	// determine compression
	switch (get_u32be(&rawheader[20]))
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
	if (UNEXPECTED(get_u32be(&rawheader[8]) != V4_HEADER_SIZE))
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = get_u64be(&rawheader[28]);
	m_mapoffset = 108;
	m_metaoffset = get_u64be(&rawheader[36]);
	m_hunkbytes = get_u32be(&rawheader[44]);
	m_hunkcount = get_u32be(&rawheader[24]);

	// extract parent SHA-1
	uint32_t flags = get_u32be(&rawheader[16]);
	m_allow_writes = (flags & 2) == 0;

	// determine compression
	switch (get_u32be(&rawheader[20]))
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
	if (UNEXPECTED(get_u32be(&rawheader[8]) != V5_HEADER_SIZE))
		throw std::error_condition(error::INVALID_FILE);

	// extract core info
	m_logicalbytes = get_u64be(&rawheader[32]);
	m_mapoffset = get_u64be(&rawheader[40]);
	m_metaoffset = get_u64be(&rawheader[48]);
	m_hunkbytes = get_u32be(&rawheader[56]);
	m_hunkcount = (m_logicalbytes + m_hunkbytes - 1) / m_hunkbytes;
	m_unitbytes = get_u32be(&rawheader[60]);
	m_unitcount = (m_logicalbytes + m_unitbytes - 1) / m_unitbytes;

	// determine compression
	m_compression[0] = get_u32be(&rawheader[16]);
	m_compression[1] = get_u32be(&rawheader[20]);
	m_compression[2] = get_u32be(&rawheader[24]);
	m_compression[3] = get_u32be(&rawheader[28]);

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
		for (uint32_t hunknum = 0; hunknum < m_hunkcount; hunknum++)
		{
			uint8_t curcomp = m_rawmap[hunknum * 12 + 0];

			if (curcomp == COMPRESSION_SELF)
			{
				// promote self block references to more compact forms
				uint32_t refhunk = get_u48be(&m_rawmap[hunknum * 12 + 4]);
				if (refhunk == last_self)
					curcomp = COMPRESSION_SELF_0;
				else if (refhunk == last_self + 1)
					curcomp = COMPRESSION_SELF_1;
				else
					max_self = std::max(max_self, refhunk);
				last_self = refhunk;
			}
			else if (curcomp == COMPRESSION_PARENT)
			{
				// promote parent block references to more compact forms
				uint32_t refunit = get_u48be(&m_rawmap[hunknum * 12 + 4]);
				if (refunit == mulu_32x32(hunknum, m_hunkbytes) / m_unitbytes)
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
				max_complen = std::max<uint32_t>(max_complen, get_u24be(&m_rawmap[hunknum * 12 + 1]));

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

		// determine the number of bits we need to hold the a length and a hunk index
		const uint8_t lengthbits = bits_for_value(max_complen);
		const uint8_t selfbits = bits_for_value(max_self);
		const uint8_t parentbits = bits_for_value(max_parent);

		// determine the needed size of the output buffer
		// 16 bytes is required for the header
		// max len per entry given to huffman encoder at instantiation is 8 bits
		// this corresponds to worst-case max 12 bits per entry when RLE encoded.
		// max additional bits per entry after RLE encoded tree is
		// for COMPRESSION_TYPE_0-3: lengthbits+16
		// for COMPRESSION_NONE: 16
		// for COMPRESSION_SELF: selfbits
		// for COMPRESSION_PARENT: parentbits
		// the overall size is clamped later with bitbuf.flush()
		int nbits_needed = (8*16) + (12 + std::max<int>({lengthbits+16, selfbits, parentbits}))*m_hunkcount;
		std::vector<uint8_t> compressed(nbits_needed / 8 + 1);
		bitstream_out bitbuf(&compressed[16], compressed.size() - 16);

		// compute a tree and export it to the buffer
		huffman_error err = encoder.compute_tree_from_histo();
		if (UNEXPECTED(err != HUFFERR_NONE))
			throw std::error_condition(error::COMPRESSION_ERROR);
		err = encoder.export_tree_rle(bitbuf);
		if (UNEXPECTED(err != HUFFERR_NONE))
			throw std::error_condition(error::COMPRESSION_ERROR);

		// encode the data
		for (uint8_t *src = &compression_rle[0]; src < dest; src++)
			encoder.encode_one(bitbuf, *src);

		// for each compression type, output the relevant data
		lastcomp = 0;
		count = 0;
		uint8_t *src = &compression_rle[0];
		uint64_t firstoffs = 0;
		for (uint32_t hunknum = 0; hunknum < m_hunkcount; hunknum++)
		{
			uint8_t *rawmap = &m_rawmap[hunknum * 12];
			uint32_t length = get_u24be(&rawmap[1]);
			uint64_t offset = get_u48be(&rawmap[4]);
			uint16_t crc = get_u16be(&rawmap[10]);

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
		put_u32be(&compressed[0], complen);
		put_u48be(&compressed[4], firstoffs);
		put_u16be(&compressed[10], mapcrc);
		compressed[12] = lengthbits;
		compressed[13] = selfbits;
		compressed[14] = parentbits;
		compressed[15] = 0;

		// write the result
		m_mapoffset = file_append(&compressed[0], complen + 16);

		// then write the map offset
		uint8_t rawbuf[sizeof(uint64_t)];
		put_u64be(rawbuf, m_mapoffset);
		return file_write(m_mapoffset_offset, rawbuf, sizeof(rawbuf));
	}
	catch (std::error_condition const &err)
	{
		return err;
	}
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
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
	std::error_condition ioerr;
	ioerr = file_read(m_mapoffset, rawbuf, sizeof(rawbuf));
	if (UNEXPECTED(ioerr))
		throw ioerr;
	uint32_t const mapbytes = get_u32be(&rawbuf[0]);
	uint64_t const firstoffs = get_u48be(&rawbuf[4]);
	util::crc16_t const mapcrc = get_u16be(&rawbuf[10]);
	uint8_t const lengthbits = rawbuf[12];
	uint8_t const selfbits = rawbuf[13];
	uint8_t const parentbits = rawbuf[14];

	// now read the map
	std::vector<uint8_t> compressed(mapbytes);
	ioerr = file_read(m_mapoffset + 16, &compressed[0], mapbytes);
	if (UNEXPECTED(ioerr))
		throw ioerr;
	bitstream_in bitbuf(&compressed[0], compressed.size());

	// first decode the compression types
	huffman_decoder<16, 8> decoder;
	huffman_error const huferr = decoder.import_tree_rle(bitbuf);
	if (UNEXPECTED(huferr != HUFFERR_NONE))
		throw std::error_condition(error::DECOMPRESSION_ERROR);
	uint8_t lastcomp = 0;
	int repcount = 0;
	for (uint32_t hunknum = 0; hunknum < m_hunkcount; hunknum++)
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
	for (uint32_t hunknum = 0; hunknum < m_hunkcount; hunknum++)
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
				last_parent = offset = mulu_32x32(hunknum, m_hunkbytes) / m_unitbytes;
				break;

			case COMPRESSION_PARENT_1:
				last_parent += m_hunkbytes / m_unitbytes;
				[[fallthrough]];
			case COMPRESSION_PARENT_0:
				rawmap[0] = COMPRESSION_PARENT;
				offset = last_parent;
				break;
		}
		put_u24be(&rawmap[1], length);
		put_u48be(&rawmap[4], offset);
		put_u16be(&rawmap[10], crc);
	}

	// verify the final CRC
	if (UNEXPECTED(util::crc16_creator::simple(&m_rawmap[0], m_hunkcount * 12) != mapcrc))
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
		if (UNEXPECTED(m_parent && m_parent->version() < 3))
			throw std::error_condition(error::UNSUPPORTED_VERSION);

		// must be an even number of units per hunk
		if (UNEXPECTED(m_hunkbytes % m_unitbytes != 0))
			throw std::error_condition(std::errc::invalid_argument);
		if (UNEXPECTED(m_parent && m_unitbytes != m_parent->unit_bytes()))
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
		put_u32be(&rawheader[8], V5_HEADER_SIZE);
		put_u32be(&rawheader[12], m_version);
		put_u32be(&rawheader[16], m_compression[0]);
		put_u32be(&rawheader[20], m_compression[1]);
		put_u32be(&rawheader[24], m_compression[2]);
		put_u32be(&rawheader[28], m_compression[3]);
		put_u64be(&rawheader[32], m_logicalbytes);
		put_u64be(&rawheader[40], compressed() ? 0 : V5_HEADER_SIZE);
		put_u64be(&rawheader[48], m_metaoffset);
		put_u32be(&rawheader[56], m_hunkbytes);
		put_u32be(&rawheader[60], m_unitbytes);
		be_write_sha1(&rawheader[64], util::sha1_t::null);
		be_write_sha1(&rawheader[84], util::sha1_t::null);
		be_write_sha1(&rawheader[104], m_parent ? m_parent->sha1() : util::sha1_t::null);

		// write the resulting header
		std::error_condition err = file_write(0, rawheader, sizeof(rawheader));
		if (UNEXPECTED(err))
			throw err;

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
				uint32_t const bytes_to_write = std::min<size_t>(mapsize, sizeof(buffer));
				err = file_write(offset, buffer, bytes_to_write);
				if (UNEXPECTED(err))
					throw err;
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

std::error_condition chd_file::open_common(bool writeable, const open_parent_func &open_parent)
{
	// wrap in try for proper error handling
	try
	{
		// reads are always permitted
		m_allow_reads = true;

		// read the raw header
		uint8_t rawheader[MAX_HEADER_SIZE];
		std::error_condition err = file_read(0, rawheader, sizeof(rawheader));
		if (UNEXPECTED(err))
			throw err;

		// verify the signature
		if (UNEXPECTED(memcmp(rawheader, "MComprHD", 8) != 0))
			throw std::error_condition(error::INVALID_FILE);

		m_version = get_u32be(&rawheader[12]);

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

		if (UNEXPECTED(writeable && !m_allow_writes))
			throw std::error_condition(error::FILE_NOT_WRITEABLE);

		// make sure we have a parent if we need one (and don't if we don't)
		if (parentsha1 != util::sha1_t::null)
		{
			if (!m_parent && open_parent)
				m_parent = open_parent(parentsha1);

			if (!m_parent)
				m_parent_missing = true;
			else if (m_parent->sha1() != parentsha1)
				throw std::error_condition(error::INVALID_PARENT);
		}
		else if (UNEXPECTED(m_parent))
		{
			throw std::error_condition(std::errc::invalid_argument);
		}

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
		if (UNEXPECTED(!m_decompressor[decompnum] && (m_compression[decompnum] != 0)))
			throw std::error_condition(error::UNKNOWN_COMPRESSION);
	}

	// read the map; v5+ compressed drives need to read and decompress their map
	m_rawmap.resize(m_hunkcount * m_mapentrybytes);
	if (m_version >= 5 && compressed())
	{
		decompress_v5_map();
	}
	else
	{
		std::error_condition err = file_read(m_mapoffset, &m_rawmap[0], m_rawmap.size());
		if (UNEXPECTED(err))
			throw err;
	}

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
 * @param   hunknum The hunknum.
 */

std::error_condition chd_file::verify_proper_compression_append(uint32_t hunknum) const noexcept
{
	// punt if no file
	if (UNEXPECTED(!m_file))
		return std::error_condition(error::NOT_OPEN);

	// return an error if out of range
	if (UNEXPECTED(hunknum >= m_hunkcount))
		return std::error_condition(error::HUNK_OUT_OF_RANGE);

	// if not writeable, fail
	if (UNEXPECTED(!m_allow_writes))
		return std::error_condition(error::FILE_NOT_WRITEABLE);

	// compressed writes only via this interface
	if (UNEXPECTED(!compressed()))
		return std::error_condition(error::FILE_NOT_WRITEABLE);

	// only permitted to write new blocks
	uint8_t const *const rawmap = &m_rawmap[hunknum * 12];
	if (UNEXPECTED(rawmap[0] != 0xff))
		return std::error_condition(error::COMPRESSION_ERROR);

	// if this isn't the first block, only permitted to write immediately after the previous one
	if (UNEXPECTED((hunknum != 0) && (rawmap[-12] == 0xff)))
		return std::error_condition(error::COMPRESSION_ERROR);

	return std::error_condition();
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
	std::error_condition err = verify_proper_compression_append(hunknum);
	if (UNEXPECTED(err))
		throw err;

	// write the final result
	uint64_t offset = file_append(compressed, complength);

	// update the map entry
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = (compression == -1) ? COMPRESSION_NONE : compression;
	put_u24be(&rawmap[1], complength);
	put_u48be(&rawmap[4], offset);
	put_u16be(&rawmap[10], crc16);
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
	std::error_condition err = verify_proper_compression_append(hunknum);
	if (UNEXPECTED(err))
		throw err;

	// only permitted to reference prior hunks
	if (UNEXPECTED(otherhunk >= hunknum))
		throw std::error_condition(error::HUNK_OUT_OF_RANGE);

	// update the map entry
	uint8_t *rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = COMPRESSION_SELF;
	put_u24be(&rawmap[1], 0);
	put_u48be(&rawmap[4], otherhunk);
	put_u16be(&rawmap[10], 0);
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
	std::error_condition err = verify_proper_compression_append(hunknum);
	if (UNEXPECTED(err))
		throw err;

	// update the map entry
	uint8_t *const rawmap = &m_rawmap[hunknum * 12];
	rawmap[0] = COMPRESSION_PARENT;
	put_u24be(&rawmap[1], 0);
	put_u48be(&rawmap[4], parentunit);
	put_u16be(&rawmap[10], 0);
}

/**
 * @fn  std::error_condition chd_file::metadata_find(chd_metadata_tag metatag, int32_t metaindex, metadata_entry &metaentry, bool resume)
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
 * @return  A std::error_condition (error::METADATA_NOT_FOUND if the search fails).
 */

std::error_condition chd_file::metadata_find(chd_metadata_tag metatag, int32_t metaindex, metadata_entry &metaentry, bool resume) const noexcept
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
		std::error_condition err = file_read(metaentry.offset, raw_meta_header, sizeof(raw_meta_header));
		if (UNEXPECTED(err))
			return err;

		// extract the data
		metaentry.metatag = get_u32be(&raw_meta_header[0]);
		metaentry.flags = raw_meta_header[4];
		metaentry.length = get_u24be(&raw_meta_header[5]);
		metaentry.next = get_u64be(&raw_meta_header[8]);

		// if we got a match, proceed
		if (metatag == CHDMETATAG_WILDCARD || metaentry.metatag == metatag)
			if (metaindex-- == 0)
				return std::error_condition();

		// no match, fetch the next link
		metaentry.prev = metaentry.offset;
		metaentry.offset = metaentry.next;
	}

	// if we get here, we didn't find it
	return error::METADATA_NOT_FOUND;
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

std::error_condition chd_file::metadata_set_previous_next(uint64_t prevoffset, uint64_t nextoffset) noexcept
{
	uint64_t offset = 0;

	if (prevoffset == 0)
	{
		// if we were the first entry, make the next entry the first
		offset = m_metaoffset_offset;
		m_metaoffset = nextoffset;
	}
	else
	{
		// otherwise, update the link in the previous header
		offset = prevoffset + 8;
	}

	// create a big-endian version
	uint8_t rawbuf[sizeof(uint64_t)];
	put_u64be(rawbuf, nextoffset);

	// write to the header and update our local copy
	return file_write(offset, rawbuf, sizeof(rawbuf));
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
	if ((m_version < 4) || !compressed())
		return;

	// compute the new overall hash
	util::sha1_t fullsha1 = compute_overall_sha1(raw_sha1());

	// create a big-endian version
	uint8_t rawbuf[sizeof(util::sha1_t)];
	be_write_sha1(&rawbuf[0], fullsha1);

	// write to the header
	std::error_condition err = file_write(m_sha1_offset, rawbuf, sizeof(rawbuf));
	if (UNEXPECTED(err))
		throw err;
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

chd_file_compressor::chd_file_compressor() :
	m_walking_parent(false),
	m_total_in(0),
	m_total_out(0),
	m_read_queue(nullptr),
	m_read_queue_offset(0),
	m_read_done_offset(0),
	m_work_queue(nullptr),
	m_write_hunk(0)
{
	// zap arrays
	std::fill(std::begin(m_codecs), std::end(m_codecs), nullptr);

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
	m_walking_parent = bool(m_parent);
	m_total_in = 0;
	m_total_out = 0;
	m_compsha1.reset();

	// reset our maps
	m_parent_map.reset();
	m_current_map.reset();

	// reset read state
	m_read_queue_offset = 0;
	m_read_done_offset = 0;
	m_read_error.clear();

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
	// if we got an error, return the error
	if (UNEXPECTED(m_read_error))
		return m_read_error;

	// if done reading, queue some more
	while (m_read_queue_offset < m_logicalbytes && osd_work_queue_items(m_read_queue) < 2)
	{
		// see if we have enough free work items to read the next half of a buffer
		uint32_t const startitem = m_read_queue_offset / hunk_bytes();
		uint32_t const enditem = startitem + WORK_BUFFER_HUNKS / 2;
		uint32_t curitem = startitem;
		while ((curitem < enditem) && (m_work_item[curitem % WORK_BUFFER_HUNKS].m_status == WS_READY))
			++curitem;

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
		if (item.m_osd)
		{
			osd_work_item_release(item.m_osd);
			item.m_osd = nullptr;
		}

		if (m_walking_parent)
		{
			// for parent walking, just add to the hashmap
			uint32_t const uph = hunk_bytes() / unit_bytes();
			uint32_t units = uph;
			if (item.m_hunknum == hunk_count() - 1 || !compressed())
				units = 1;
			for (uint32_t unit = 0; unit < units; unit++)
			{
				if (m_parent_map.find(item.m_hash[unit].m_crc16, item.m_hash[unit].m_sha1) == hashmap::NOT_FOUND)
					m_parent_map.add(item.m_hunknum * uph + unit, item.m_hash[unit].m_crc16, item.m_hash[unit].m_sha1);
			}
		}
		else if (!compressed())
		{
			// if we're uncompressed, use regular writes
			std::error_condition err = write_hunk(item.m_hunknum, item.m_data);
			if (UNEXPECTED(err))
				return err;

			// writes of all-0 data don't actually take space, so see if we count this
			chd_codec_type codec = CHD_CODEC_NONE;
			uint32_t complen;
			err = hunk_info(item.m_hunknum, codec, complen);
			if (!err && codec == CHD_CODEC_NONE) // TODO: report error?
				m_total_out += m_hunkbytes;
		}
		else if (uint64_t const selfhunk = m_current_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1); selfhunk != hashmap::NOT_FOUND)
		{
			// the hunk is in the self map
			hunk_copy_from_self(item.m_hunknum, selfhunk);
		}
		else
		{
			// if not, see if it's in the parent map
			uint64_t const parentunit = m_parent ? m_parent_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1) : hashmap::NOT_FOUND;
			if (parentunit != hashmap::NOT_FOUND)
			{
				hunk_copy_from_parent(item.m_hunknum, parentunit);
			}
			else
			{
				// otherwise, append it compressed and add to the self map
				hunk_write_compressed(item.m_hunknum, item.m_compression, item.m_compressed, item.m_complen, item.m_hash[0].m_crc16);
				m_total_out += item.m_complen;
				m_current_map.add(item.m_hunknum, item.m_hash[0].m_crc16, item.m_hash[0].m_sha1);
			}
		}

		// reset the item and advance
		item.m_status = WS_READY;
		m_write_hunk++;

		// if we hit the end, finalize
		if (m_write_hunk == m_hunkcount)
		{
			if (m_walking_parent)
			{
				// if this is just walking the parent, reset and get ready for compression
				m_walking_parent = false;
				m_read_queue_offset = m_read_done_offset = 0;
				m_write_hunk = 0;
				for (auto &elem : m_work_item)
					elem.m_status = WS_READY;
			}
			else
			{
				// wait for all reads to finish and if we're compressed, write the final SHA1 and map
				osd_work_queue_wait(m_read_queue, 30 * osd_ticks_per_second());
				if (!compressed())
					return std::error_condition();
				std::error_condition err = set_raw_sha1(m_compsha1.finish());
				if (UNEXPECTED(err))
					return err;
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
	while ((m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_status != WS_READY) &&
			(m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_status != WS_COMPLETE) &&
			m_work_item[m_write_hunk % WORK_BUFFER_HUNKS].m_osd)
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
	auto *const item = reinterpret_cast<work_item *>(param);
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
	auto *const item = reinterpret_cast<work_item *>(param);
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
	if ((m_current_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1) == hashmap::NOT_FOUND) &&
			(m_parent_map.find(item.m_hash[0].m_crc16, item.m_hash[0].m_sha1) == hashmap::NOT_FOUND))
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
	if (UNEXPECTED(m_read_error))
		return;

	// determine parameters for the read
	uint32_t const work_buffer_bytes = WORK_BUFFER_HUNKS * hunk_bytes();
	uint32_t numbytes = work_buffer_bytes / 2;
	if ((m_read_done_offset + numbytes) > logical_bytes())
		numbytes = logical_bytes() - m_read_done_offset;

	uint8_t *const dest = &m_work_buffer[0] + (m_read_done_offset % work_buffer_bytes);
	assert((&m_work_buffer[0] == dest) || (&m_work_buffer[work_buffer_bytes / 2] == dest));
	assert(!(m_read_done_offset % hunk_bytes()));
	uint64_t const end_offset = m_read_done_offset + numbytes;

	// catch any exceptions coming out of here
	try
	{
		// do the read
		if (m_walking_parent)
		{
			// if walking the parent, read in hunks from the parent CHD
			uint64_t curoffs = m_read_done_offset;
			uint8_t *curdest = dest;
			uint32_t curhunk = m_read_done_offset / hunk_bytes();
			while (curoffs < end_offset + 1)
			{
				std::error_condition err = m_parent->read_hunk(curhunk, curdest);
				if (err && (error::HUNK_OUT_OF_RANGE != err)) // FIXME: fix the code so it doesn't depend on trying to read past the end of the parent CHD
					throw err;
				curoffs += hunk_bytes();
				curdest += hunk_bytes();
				++curhunk;
			}
		}
		else
		{
			// otherwise, call the virtual function
			read_data(dest, m_read_done_offset, numbytes);
		}

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
		m_read_error = err;
	}
	catch (std::exception const &ex)
	{
		fprintf(stderr, "exception occurred: %s\n", ex.what());
		m_read_error = std::errc::io_error; // TODO: revisit this error code
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

chd_file_compressor::hashmap::hashmap() :
	m_block_list(new entry_block(nullptr))
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

uint64_t chd_file_compressor::hashmap::find(util::crc16_t crc16, util::sha1_t sha1) const noexcept
{
	// look up the entry in the map
	for (entry_t *entry = m_map[crc16]; entry; entry = entry->m_next)
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

std::error_condition chd_file::check_is_hd() const noexcept
{
	metadata_entry metaentry;
	return metadata_find(HARD_DISK_METADATA_TAG, 0, metaentry);
}

std::error_condition chd_file::check_is_cd() const noexcept
{
	metadata_entry metaentry;
	std::error_condition err = metadata_find(CDROM_OLD_METADATA_TAG, 0, metaentry);
	if (err == error::METADATA_NOT_FOUND)
		err = metadata_find(CDROM_TRACK_METADATA_TAG, 0, metaentry);
	if (err == error::METADATA_NOT_FOUND)
		err = metadata_find(CDROM_TRACK_METADATA2_TAG, 0, metaentry);
	return err;
}

std::error_condition chd_file::check_is_gd() const noexcept
{
	metadata_entry metaentry;
	std::error_condition err = metadata_find(GDROM_OLD_METADATA_TAG, 0, metaentry);
	if (err == error::METADATA_NOT_FOUND)
		err = metadata_find(GDROM_TRACK_METADATA_TAG, 0, metaentry);
	return err;
}

std::error_condition chd_file::check_is_dvd() const noexcept
{
	metadata_entry metaentry;
	return metadata_find(DVD_METADATA_TAG, 0, metaentry);
}

std::error_condition chd_file::check_is_av() const noexcept
{
	metadata_entry metaentry;
	return metadata_find(AV_METADATA_TAG, 0, metaentry);
}
