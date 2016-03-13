// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.c

    Functions to manipulate data within ZIP files.

***************************************************************************/

#include "osdcore.h"
#include "unzip.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <utility>
#include <vector>

#include <zlib.h>



namespace {
/***************************************************************************
    CONSTANTS
***************************************************************************/

/* offsets in end of central directory structure */
#define ZIPESIG         0x00
#define ZIPEDSK         0x04
#define ZIPECEN         0x06
#define ZIPENUM         0x08
#define ZIPECENN        0x0a
#define ZIPECSZ         0x0c
#define ZIPEOFST        0x10
#define ZIPECOML        0x14
#define ZIPECOM         0x16

/* offsets in central directory entry structure */
#define ZIPCENSIG       0x00
#define ZIPCVER         0x04
#define ZIPCOS          0x05
#define ZIPCVXT         0x06
#define ZIPCEXOS        0x07
#define ZIPCFLG         0x08
#define ZIPCMTHD        0x0a
#define ZIPCTIM         0x0c
#define ZIPCDAT         0x0e
#define ZIPCCRC         0x10
#define ZIPCSIZ         0x14
#define ZIPCUNC         0x18
#define ZIPCFNL         0x1c
#define ZIPCXTL         0x1e
#define ZIPCCML         0x20
#define ZIPDSK          0x22
#define ZIPINT          0x24
#define ZIPEXT          0x26
#define ZIPOFST         0x2a
#define ZIPCFN          0x2e

/* offsets in local file header structure */
#define ZIPLOCSIG       0x00
#define ZIPVER          0x04
#define ZIPGENFLG       0x06
#define ZIPMTHD         0x08
#define ZIPTIME         0x0a
#define ZIPDATE         0x0c
#define ZIPCRC          0x0e
#define ZIPSIZE         0x12
#define ZIPUNCMP        0x16

/**
 * @def ZIPFNLN
 *
 * @brief   A macro that defines zipfnln.
 */

#define ZIPFNLN         0x1a

/**
 * @def ZIPXTRALN
 *
 * @brief   A macro that defines zipxtraln.
 */

#define ZIPXTRALN       0x1c

/**
 * @def ZIPNAME
 *
 * @brief   A macro that defines zipname.
 */

#define ZIPNAME         0x1e



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class zip_file_impl
{
public:
	typedef std::unique_ptr<zip_file_impl> ptr;

	zip_file_impl(const std::string &filename)
		: m_filename(filename)
		, m_file()
		, m_length(0)
		, m_ecd()
		, m_cd()
		, m_cd_pos(0)
		, m_header()
		, m_buffer()
	{
		std::memset(&m_header, 0, sizeof(m_header));
		std::fill(m_buffer.begin(), m_buffer.end(), 0);
	}

	static ptr find_cached(const std::string &filename)
	{
		for (std::size_t cachenum = 0; cachenum < s_cache.size(); cachenum++)
		{
			// if we have a valid entry and it matches our filename, use it and remove from the cache
			if (s_cache[cachenum] && (filename == s_cache[cachenum]->m_filename))
			{
				ptr result;
				std::swap(s_cache[cachenum], result);
				return result;
			}
		}
		return ptr();
	}
	static void close(ptr &&zip);
	static void cache_clear()
	{
		// clear call cache entries
		for (std::size_t cachenum = 0; cachenum < s_cache.size(); s_cache[cachenum++].reset()) { }
	}

	zip_file::error initialize()
	{
		// read ecd data
		auto const ziperr = read_ecd();
		if (ziperr != zip_file::error::NONE)
			return ziperr;

		// verify that we can work with this zipfile (no disk spanning allowed)
		if ((m_ecd.disk_number != m_ecd.cd_start_disk_number) || (m_ecd.cd_disk_entries != m_ecd.cd_total_entries))
			return zip_file::error::UNSUPPORTED;

		// allocate memory for the central directory
		try { m_cd.resize(m_ecd.cd_size + 1); }
		catch (...) { return zip_file::error::OUT_OF_MEMORY; }

		// read the central directory
		std::uint32_t read_length;
		auto const filerr = m_file->read(&m_cd[0], m_ecd.cd_start_disk_offset, m_ecd.cd_size, read_length);
		if ((filerr != osd_file::error::NONE) || (read_length != m_ecd.cd_size))
			return (filerr == osd_file::error::NONE) ? zip_file::error::FILE_TRUNCATED : zip_file::error::FILE_ERROR;

		return zip_file::error::NONE;
	}

	// contained file access
	const zip_file::file_header *first_file();
	const zip_file::file_header *next_file();
	zip_file::error decompress(void *buffer, std::uint32_t length);

private:
	zip_file_impl(const zip_file_impl &) = delete;
	zip_file_impl(zip_file_impl &&) = delete;
	zip_file_impl &operator=(const zip_file_impl &) = delete;
	zip_file_impl &operator=(zip_file_impl &&) = delete;

	zip_file::error reopen()
	{
		if (!m_file)
		{
			auto const filerr = osd_file::open(m_filename, OPEN_FLAG_READ, m_file, m_length);
			if (filerr != osd_file::error::NONE)
				return zip_file::error::FILE_ERROR;
		}
		return zip_file::error::NONE;
	}

	// ZIP file parsing
	zip_file::error read_ecd();
	zip_file::error get_compressed_data_offset(std::uint64_t &offset);

	// decompression interfaces
	zip_file::error decompress_data_type_0(std::uint64_t offset, void *buffer, std::uint32_t length);
	zip_file::error decompress_data_type_8(std::uint64_t offset, void *buffer, std::uint32_t length);

	struct file_header_int : zip_file::file_header
	{
		std::uint8_t *  raw;                    // pointer to the raw data
		std::uint32_t   rawlength;              // length of the raw data
		std::uint8_t    saved;                  // saved byte from after filename
	};

	// contains extracted end of central directory information
	struct ecd
	{
		std::uint32_t   signature;              // end of central dir signature
		std::uint16_t   disk_number;            // number of this disk
		std::uint16_t   cd_start_disk_number;   // number of the disk with the start of the central directory
		std::uint16_t   cd_disk_entries;        // total number of entries in the central directory on this disk
		std::uint16_t   cd_total_entries;       // total number of entries in the central directory
		std::uint32_t   cd_size;                // size of the central directory
		std::uint32_t   cd_start_disk_offset;   // offset of start of central directory with respect to the starting disk number
		std::uint16_t   comment_length;         // .ZIP file comment length
		const char *    comment;                // .ZIP file comment

		std::unique_ptr<std::uint8_t []> raw;   // pointer to the raw data
		std::uint32_t   rawlength;              // length of the raw data
	};

	static constexpr std::size_t DECOMPRESS_BUFSIZE = 16384;
	static constexpr std::size_t CACHE_SIZE = 8; // number of open files to cache
	static std::array<ptr, CACHE_SIZE> s_cache;

	const std::string           m_filename;                 // copy of ZIP filename (for caching)
	osd_file::ptr               m_file;                     // OSD file handle
	std::uint64_t               m_length;                   // length of zip file

	ecd                         m_ecd;                      // end of central directory

	std::vector<std::uint8_t>   m_cd;                       // central directory raw data
	std::uint32_t               m_cd_pos;                   // position in central directory
	file_header_int             m_header;                   // current file header

	std::array<std::uint8_t, DECOMPRESS_BUFSIZE> m_buffer;  // buffer for decompression
};


class zip_file_wrapper : public zip_file
{
public:
	zip_file_wrapper(zip_file_impl::ptr &&impl) : m_impl(std::move(impl)) { assert(m_impl); }
	virtual ~zip_file_wrapper() { zip_file_impl::close(std::move(m_impl)); }

	virtual const file_header *first_file() override { return m_impl->first_file(); }
	virtual const file_header *next_file() override { return m_impl->next_file(); }
	virtual error decompress(void *buffer, std::uint32_t length) override { return m_impl->decompress(buffer, length); }

private:
	zip_file_impl::ptr m_impl;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/**
 * @fn  static inline UINT16 read_word(UINT8 *buf)
 *
 * @brief   Reads a word.
 *
 * @param [in,out]  buf If non-null, the buffer.
 *
 * @return  The word.
 */

inline UINT16 read_word(UINT8 *buf)
{
	return (buf[1] << 8) | buf[0];
}

/**
 * @fn  static inline UINT32 read_dword(UINT8 *buf)
 *
 * @brief   Reads a double word.
 *
 * @param [in,out]  buf If non-null, the buffer.
 *
 * @return  The double word.
 */

inline UINT32 read_dword(UINT8 *buf)
{
	return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/** @brief  The zip cache[ zip cache size]. */
std::array<zip_file_impl::ptr, zip_file_impl::CACHE_SIZE> zip_file_impl::s_cache;



/*-------------------------------------------------
    zip_file_close - close a ZIP file and add it
    to the cache
-------------------------------------------------*/

/**
 * @fn  void zip_file_close(zip_file *zip)
 *
 * @brief   Zip file close.
 *
 * @param [in,out]  zip If non-null, the zip.
 */

void zip_file_impl::close(ptr &&zip)
{
	if (!zip) return;

	// close the open files
	zip->m_file.reset();

	// find the first NULL entry in the cache
	std::size_t cachenum;
	for (cachenum = 0; cachenum < s_cache.size(); cachenum++)
		if (!s_cache[cachenum])
			break;

	// if no room left in the cache, free the bottommost entry
	if (cachenum == s_cache.size())
		s_cache[--cachenum].reset();

	// move everyone else down and place us at the top
	for ( ; cachenum > 0; cachenum--)
		s_cache[cachenum] = std::move(s_cache[cachenum - 1]);
	s_cache[0] = std::move(zip);
}


/***************************************************************************
    CONTAINED FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_first_entry - return the first entry
    in the ZIP
-------------------------------------------------*/

/**
 * @fn  const zip_file_header *zip_file_first_file(zip_file *zip)
 *
 * @brief   Zip file first file.
 *
 * @param [in,out]  zip If non-null, the zip.
 *
 * @return  null if it fails, else a zip_file_header*.
 */

const zip_file::file_header *zip_file_impl::first_file()
{
	/* reset the position and go from there */
	m_cd_pos = 0;
	return next_file();
}


/*-------------------------------------------------
    zip_file_next_entry - return the next entry
    in the ZIP
-------------------------------------------------*/

/**
 * @fn  const zip_file_header *zip_file_next_file(zip_file *zip)
 *
 * @brief   Zip file next file.
 *
 * @param [in,out]  zip If non-null, the zip.
 *
 * @return  null if it fails, else a zip_file_header*.
 */

const zip_file::file_header *zip_file_impl::next_file()
{
	// fix up any modified data
	if (m_header.raw)
	{
		m_header.raw[ZIPCFN + m_header.filename_length] = m_header.saved;
		m_header.raw = nullptr;
	}

	// if we're at or past the end, we're done
	if (m_cd_pos >= m_ecd.cd_size)
		return nullptr;

	// extract file header info
	m_header.raw                 = &m_cd[0] + m_cd_pos;
	m_header.rawlength           = ZIPCFN;
	m_header.signature           = read_dword(m_header.raw + ZIPCENSIG);
	m_header.version_created     = read_word (m_header.raw + ZIPCVER);
	m_header.version_needed      = read_word (m_header.raw + ZIPCVXT);
	m_header.bit_flag            = read_word (m_header.raw + ZIPCFLG);
	m_header.compression         = read_word (m_header.raw + ZIPCMTHD);
	m_header.file_time           = read_word (m_header.raw + ZIPCTIM);
	m_header.file_date           = read_word (m_header.raw + ZIPCDAT);
	m_header.crc                 = read_dword(m_header.raw + ZIPCCRC);
	m_header.compressed_length   = read_dword(m_header.raw + ZIPCSIZ);
	m_header.uncompressed_length = read_dword(m_header.raw + ZIPCUNC);
	m_header.filename_length     = read_word (m_header.raw + ZIPCFNL);
	m_header.extra_field_length  = read_word (m_header.raw + ZIPCXTL);
	m_header.file_comment_length = read_word (m_header.raw + ZIPCCML);
	m_header.start_disk_number   = read_word (m_header.raw + ZIPDSK);
	m_header.internal_attributes = read_word (m_header.raw + ZIPINT);
	m_header.external_attributes = read_dword(m_header.raw + ZIPEXT);
	m_header.local_header_offset = read_dword(m_header.raw + ZIPOFST);
	m_header.filename            = reinterpret_cast<const char *>(m_header.raw + ZIPCFN);

	// make sure we have enough data
	m_header.rawlength += m_header.filename_length;
	m_header.rawlength += m_header.extra_field_length;
	m_header.rawlength += m_header.file_comment_length;
	if (m_cd_pos + m_header.rawlength > m_ecd.cd_size)
		return nullptr;

	// NULL terminate the filename
	m_header.saved = m_header.raw[ZIPCFN + m_header.filename_length];
	m_header.raw[ZIPCFN + m_header.filename_length] = 0;

	// advance the position
	m_cd_pos += m_header.rawlength;
	return &m_header;
}


/*-------------------------------------------------
    zip_file_decompress - decompress a file
    from a ZIP into the target buffer
-------------------------------------------------*/

/**
 * @fn  zip_error zip_file_decompress(zip_file *zip, void *buffer, UINT32 length)
 *
 * @brief   Zip file decompress.
 *
 * @param [in,out]  zip     If non-null, the zip.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   length          The length.
 *
 * @return  A zip_error.
 */

zip_file::error zip_file_impl::decompress(void *buffer, UINT32 length)
{
	zip_file::error ziperr;
	std::uint64_t offset;

	// if we don't have enough buffer, error
	if (length < m_header.uncompressed_length)
		return zip_file::error::BUFFER_TOO_SMALL;

	// make sure the info in the header aligns with what we know
	if (m_header.start_disk_number != m_ecd.disk_number)
		return zip_file::error::UNSUPPORTED;

	// get the compressed data offset
	ziperr = get_compressed_data_offset(offset);
	if (ziperr != zip_file::error::NONE)
		return ziperr;

	// handle compression types
	switch (m_header.compression)
	{
	case 0:
		ziperr = decompress_data_type_0(offset, buffer, length);
		break;

	case 8:
		ziperr = decompress_data_type_8(offset, buffer, length);
		break;

	default:
		ziperr = zip_file::error::UNSUPPORTED;
		break;
	}
	return ziperr;
}



/***************************************************************************
    ZIP FILE PARSING
***************************************************************************/

/*-------------------------------------------------
    read_ecd - read the ECD data
-------------------------------------------------*/

/**
 * @fn  static zip_error read_ecd(zip_file *zip)
 *
 * @brief   Reads an ecd.
 *
 * @param [in,out]  zip If non-null, the zip.
 *
 * @return  The ecd.
 */

zip_file::error zip_file_impl::read_ecd()
{
	// make sure the file handle is open
	auto const ziperr = reopen();
	if (ziperr != zip_file::error::NONE)
		return ziperr;

	// we may need multiple tries
	std::uint32_t buflen = 1024;
	while (buflen < 65536)
	{
		// max out the buffer length at the size of the file
		if (buflen > m_length)
			buflen = m_length;

		// allocate buffer
		std::unique_ptr<std::uint8_t []> buffer;
		try { buffer.reset(new std::uint8_t[buflen + 1]); }
		catch (...) { return zip_file::error::OUT_OF_MEMORY; }

		// read in one buffers' worth of data
		std::uint32_t read_length;
		auto const error = m_file->read(&buffer[0], m_length - buflen, buflen, read_length);
		if (error != osd_file::error::NONE || read_length != buflen)
			return zip_file::error::FILE_ERROR;

		// find the ECD signature
		std::int32_t offset;
		for (offset = buflen - 22; offset >= 0; offset--)
			if (buffer[offset + 0] == 'P' && buffer[offset + 1] == 'K' && buffer[offset + 2] == 0x05 && buffer[offset + 3] == 0x06)
				break;

		// if we found it, fill out the data
		if (offset >= 0)
		{
			// reuse the buffer as our ECD buffer
			m_ecd.raw = std::move(buffer);
			m_ecd.rawlength = buflen - offset;

			/* append a NULL terminator to the comment */
			memmove(&m_ecd.raw[0], &m_ecd.raw[offset], m_ecd.rawlength);
			m_ecd.raw[m_ecd.rawlength] = 0;

			/* extract ecd info */
			m_ecd.signature            = read_dword(&m_ecd.raw[ZIPESIG]);
			m_ecd.disk_number          = read_word (&m_ecd.raw[ZIPEDSK]);
			m_ecd.cd_start_disk_number = read_word (&m_ecd.raw[ZIPECEN]);
			m_ecd.cd_disk_entries      = read_word (&m_ecd.raw[ZIPENUM]);
			m_ecd.cd_total_entries     = read_word (&m_ecd.raw[ZIPECENN]);
			m_ecd.cd_size              = read_dword(&m_ecd.raw[ZIPECSZ]);
			m_ecd.cd_start_disk_offset = read_dword(&m_ecd.raw[ZIPEOFST]);
			m_ecd.comment_length       = read_word (&m_ecd.raw[ZIPECOML]);
			m_ecd.comment              = reinterpret_cast<const char *>(&m_ecd.raw[ZIPECOM]);
			return zip_file::error::NONE;
		}

		// didn't find it; free this buffer and expand our search
		if (buflen < m_length)
			buflen *= 2;
		else
			return zip_file::error::BAD_SIGNATURE;
	}
	return zip_file::error::OUT_OF_MEMORY;
}


/*-------------------------------------------------
    get_compressed_data_offset - return the
    offset of the compressed data
-------------------------------------------------*/

/**
 * @fn  static zip_error get_compressed_data_offset(zip_file *zip, UINT64 *offset)
 *
 * @brief   Gets compressed data offset.
 *
 * @param [in,out]  zip     If non-null, the zip.
 * @param [in,out]  offset  If non-null, the offset.
 *
 * @return  The compressed data offset.
 */

zip_file::error zip_file_impl::get_compressed_data_offset(std::uint64_t &offset)
{
	// make sure the file handle is open
	auto const ziperr = reopen();
	if (ziperr != zip_file::error::NONE)
		return ziperr;

	// now go read the fixed-sized part of the local file header
	std::uint32_t read_length;
	auto const error = m_file->read(&m_buffer[0], m_header.local_header_offset, ZIPNAME, read_length);
	if (error != osd_file::error::NONE || read_length != ZIPNAME)
		return (error == osd_file::error::NONE) ? zip_file::error::FILE_TRUNCATED : zip_file::error::FILE_ERROR;

	/* compute the final offset */
	offset = m_header.local_header_offset + ZIPNAME;
	offset += read_word(&m_buffer[ZIPFNLN]);
	offset += read_word(&m_buffer[ZIPXTRALN]);

	return zip_file::error::NONE;
}



/***************************************************************************
    DECOMPRESSION INTERFACES
***************************************************************************/

/*-------------------------------------------------
    decompress_data_type_0 - "decompress"
    type 0 data (which is uncompressed)
-------------------------------------------------*/

/**
 * @fn  static zip_error decompress_data_type_0(zip_file *zip, UINT64 offset, void *buffer, UINT32 length)
 *
 * @brief   Decompress the data type 0.
 *
 * @param [in,out]  zip     If non-null, the zip.
 * @param   offset          The offset.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   length          The length.
 *
 * @return  A zip_error.
 */

zip_file::error zip_file_impl::decompress_data_type_0(std::uint64_t offset, void *buffer, std::uint32_t length)
{
	std::uint32_t read_length;

	// the data is uncompressed; just read it
	auto const filerr = m_file->read(buffer, offset, m_header.compressed_length, read_length);
	if (filerr != osd_file::error::NONE)
		return zip_file::error::FILE_ERROR;
	else if (read_length != m_header.compressed_length)
		return zip_file::error::FILE_TRUNCATED;
	else
		return zip_file::error::NONE;
}


/*-------------------------------------------------
    decompress_data_type_8 - decompress
    type 8 data (which is deflated)
-------------------------------------------------*/

/**
 * @fn  static zip_error decompress_data_type_8(zip_file *zip, UINT64 offset, void *buffer, UINT32 length)
 *
 * @brief   Decompress the data type 8.
 *
 * @param [in,out]  zip     If non-null, the zip.
 * @param   offset          The offset.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   length          The length.
 *
 * @return  A zip_error.
 */

zip_file::error zip_file_impl::decompress_data_type_8(std::uint64_t offset, void *buffer, std::uint32_t length)
{
	std::uint32_t input_remaining = m_header.compressed_length;
	int zerr;

	// make sure we don't need a newer mechanism
	if (m_header.version_needed > 0x14)
		return zip_file::error::UNSUPPORTED;

	/* reset the stream */
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_out = (Bytef *)buffer;
	stream.avail_out = length;

	// initialize the decompressor
	zerr = inflateInit2(&stream, -MAX_WBITS);
	if (zerr != Z_OK)
		return zip_file::error::DECOMPRESS_ERROR;

	// loop until we're done
	while (1)
	{
		// read in the next chunk of data
		std::uint32_t read_length;
		auto const filerr = m_file->read(&m_buffer[0], offset, (std::min<std::uint32_t>)(input_remaining, m_buffer.size()), read_length);
		if (filerr != osd_file::error::NONE)
		{
			inflateEnd(&stream);
			return zip_file::error::FILE_ERROR;
		}
		offset += read_length;

		// if we read nothing, but still have data left, the file is truncated
		if (read_length == 0 && input_remaining > 0)
		{
			inflateEnd(&stream);
			return zip_file::error::FILE_TRUNCATED;
		}

		// fill out the input data
		stream.next_in = &m_buffer[0];
		stream.avail_in = read_length;
		input_remaining -= read_length;

		// add a dummy byte at end of compressed data
		if (input_remaining == 0)
			stream.avail_in++;

		// now inflate
		zerr = inflate(&stream, Z_NO_FLUSH);
		if (zerr == Z_STREAM_END)
			break;
		if (zerr != Z_OK)
		{
			inflateEnd(&stream);
			return zip_file::error::DECOMPRESS_ERROR;
		}
	}

	// finish decompression
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
		return zip_file::error::DECOMPRESS_ERROR;

	/* if anything looks funny, report an error */
	if (stream.avail_out > 0 || input_remaining > 0)
		return zip_file::error::DECOMPRESS_ERROR;

	return zip_file::error::NONE;
}

} // anonymous namespace



/***************************************************************************
    ZIP FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_open - opens a ZIP file for reading
-------------------------------------------------*/

/**
 * @fn  zip_error zip_file_open(const char *filename, zip_file **zip)
 *
 * @brief   Queries if a given zip file open.
 *
 * @param   filename    Filename of the file.
 * @param [in,out]  zip If non-null, the zip.
 *
 * @return  A zip_error.
 */

zip_file::error zip_file::open(const std::string &filename, ptr &zip)
{
	// ensure we start with a NULL result
	zip.reset();

	// see if we are in the cache, and reopen if so
	zip_file_impl::ptr newimpl(zip_file_impl::find_cached(filename));

	if (!newimpl)
	{
		// allocate memory for the zip_file structure
		try { newimpl = std::make_unique<zip_file_impl>(filename); }
		catch (...) { return error::OUT_OF_MEMORY; }
		auto const ziperr = newimpl->initialize();
		if (ziperr != error::NONE) return ziperr;
	}

	try
	{
		zip = std::make_unique<zip_file_wrapper>(std::move(newimpl));
		return error::NONE;
	}
	catch (...)
	{
		zip_file_impl::close(std::move(newimpl));
		return error::OUT_OF_MEMORY;
	}
}


/*-------------------------------------------------
    zip_file_cache_clear - clear the ZIP file
    cache and free all memory
-------------------------------------------------*/

/**
 * @fn  void zip_file_cache_clear(void)
 *
 * @brief   Zip file cache clear.
 */

void zip_file::cache_clear()
{
	zip_file_impl::cache_clear();
}


zip_file::~zip_file()
{
}
