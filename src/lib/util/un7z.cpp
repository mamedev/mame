// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    un7z.cpp

    Functions to manipulate data within 7z files.

***************************************************************************/

// this is based on unzip.c, with modifications needed to use the 7zip library

#include "unzip.h"

#include "corestr.h"
#include "ioprocs.h"
#include "unicode.h"
#include "timeconv.h"

#include "osdcore.h"
#include "osdfile.h"

#include "lzma/C/7z.h"
#include "lzma/C/7zAlloc.h"
#include "lzma/C/7zCrc.h"
#include "lzma/C/7zTypes.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <ratio>
#include <utility>
#include <vector>


namespace util {

namespace {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct CFileInStream : public ISeekInStream
{
	CFileInStream() noexcept
	{
		Read = &CFileInStream::read_static;
		Seek = &CFileInStream::seek_static;
	}

	random_read::ptr    file;
	std::uint64_t       currfpos = 0;
	std::uint64_t       length = 0;

private:
	SRes read(void *data, std::size_t &size) noexcept
	{
		if (!file)
		{
			osd_printf_error("un7z: called CFileInStream::read without file\n");
			return SZ_ERROR_READ;
		}

		if (!size)
			return SZ_OK;

		auto const [err, read_length] = read_at(*file, currfpos, data, size);
		size = read_length;
		currfpos += read_length;

		return !err ? SZ_OK : SZ_ERROR_READ;
	}

	SRes seek(Int64 &pos, ESzSeek origin) noexcept
	{
		// need to synthesise this because the OSD file wrapper doesn't implement SEEK_END
		switch (origin)
		{
		case SZ_SEEK_CUR:
			if ((0 > pos) && (-pos > currfpos))
			{
				osd_printf_error("un7z: attemped to seek back %d bytes when current offset is %u\n", -pos, currfpos);
				return SZ_ERROR_READ;
			}
			currfpos += pos;
			break;
		case SZ_SEEK_SET:
			currfpos = pos;
			break;
		case SZ_SEEK_END:
			if ((0 > pos) && (-pos > length))
			{
				osd_printf_error("un7z: attemped to seek %d bytes before end of file %u\n", -pos, currfpos);
				return SZ_ERROR_READ;
			}
			currfpos = length + pos;
			break;
		default:
			return SZ_ERROR_READ;
		}
		pos = currfpos;
		return SZ_OK;
	}

	static SRes read_static(ISeekInStreamPtr pp, void *buf, size_t *size) noexcept
	{
		return static_cast<CFileInStream *>(const_cast<ISeekInStream *>(pp))->read(buf, *size);
	}

	static SRes seek_static(ISeekInStreamPtr pp, Int64 *pos, ESzSeek origin) noexcept
	{
		return static_cast<CFileInStream *>(const_cast<ISeekInStream *>(pp))->seek(*pos, origin);
	}
};


class m7z_file_impl
{
public:
	typedef std::unique_ptr<m7z_file_impl> ptr;

	m7z_file_impl(std::string &&filename) noexcept;

	m7z_file_impl(random_read::ptr &&file) noexcept
		: m7z_file_impl(std::string())
	{
		m_archive_stream.file = std::move(file);
	}

	virtual ~m7z_file_impl()
	{
		if (m_out_buffer)
			ISzAlloc_Free(&m_alloc_imp, m_out_buffer);
		if (m_inited)
			SzArEx_Free(&m_db, &m_alloc_imp);
	}

	static ptr find_cached(std::string_view filename) noexcept
	{
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		for (std::size_t cachenum = 0; cachenum < s_cache.size(); cachenum++)
		{
			// if we have a valid entry and it matches our filename, use it and remove from the cache
			if (s_cache[cachenum] && (filename == s_cache[cachenum]->m_filename))
			{
				using std::swap;
				ptr result;
				swap(s_cache[cachenum], result);
				osd_printf_verbose("un7z: found %s in cache\n", filename);
				return result;
			}
		}
		return ptr();
	}

	static void close(ptr &&archive) noexcept;

	static void cache_clear() noexcept
	{
		// clear call cache entries
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		for (auto &cached : s_cache)
			cached.reset();
	}

	std::error_condition initialize() noexcept;

	int first_file() noexcept
	{
		return search(0, 0, std::string_view(), false, false, false);
	}

	int next_file() noexcept
	{
		return (m_curr_file_idx < 0) ? -1 : search(m_curr_file_idx + 1, 0, std::string_view(), false, false, false);
	}

	int search(std::uint32_t crc) noexcept
	{
		return search(0, crc, std::string_view(), true, false, false);
	}

	int search(std::string_view filename, bool partialpath) noexcept
	{
		return search(0, 0, filename, false, true, partialpath);
	}

	int search(std::uint32_t crc, std::string_view filename, bool partialpath) noexcept
	{
		return search(0, crc, filename, true, true, partialpath);
	}

	bool current_is_directory() const noexcept { return m_curr_is_dir; }
	const std::string &current_name() const noexcept { return m_curr_name; }
	std::uint64_t current_uncompressed_length() const noexcept { return m_curr_length; }
	virtual std::chrono::system_clock::time_point current_last_modified() const noexcept { return m_curr_modified; }
	std::uint32_t current_crc() const noexcept { return m_curr_crc; }

	std::error_condition decompress(void *buffer, std::size_t length) noexcept;

private:
	m7z_file_impl(const m7z_file_impl &) = delete;
	m7z_file_impl(m7z_file_impl &&) = delete;
	m7z_file_impl &operator=(const m7z_file_impl &) = delete;
	m7z_file_impl &operator=(m7z_file_impl &&) = delete;

	int search(
			int i,
			std::uint32_t search_crc,
			std::string_view search_filename,
			bool matchcrc,
			bool matchname,
			bool partialpath) noexcept;
	void make_utf8_name(int index);
	void set_curr_modified() noexcept;

	static constexpr std::size_t            CACHE_SIZE = 8;
	static std::array<ptr, CACHE_SIZE>      s_cache;
	static std::mutex                       s_cache_mutex;

	const std::string                       m_filename;             // copy of _7Z filename (for caching)

	int                                     m_curr_file_idx;        // current file index
	bool                                    m_curr_is_dir;          // current file is directory
	std::string                             m_curr_name;            // current file name
	std::uint64_t                           m_curr_length;          // current file uncompressed length
	std::chrono::system_clock::time_point   m_curr_modified;        // current file modification time
	std::uint32_t                           m_curr_crc;             // current file crc

	std::vector<UInt16>                     m_utf16_buf;
	std::vector<char32_t>                   m_uchar_buf;
	std::vector<char>                       m_utf8_buf;

	CFileInStream                           m_archive_stream;
	CLookToRead2                            m_look_stream;
	CSzArEx                                 m_db;
	ISzAlloc                                m_alloc_imp;
	ISzAlloc                                m_alloc_temp_imp;
	bool                                    m_inited;

	// cached stuff for solid blocks
	UInt32                                  m_block_index;
	Byte *                                  m_out_buffer;
	std::size_t                             m_out_buffer_size;
	Byte                                    m_look_stream_buf[65'536];
};


class m7z_file_wrapper : public archive_file
{
public:
	m7z_file_wrapper(m7z_file_impl::ptr &&impl) noexcept : m_impl(std::move(impl)) { assert(m_impl); }
	virtual ~m7z_file_wrapper() override { m7z_file_impl::close(std::move(m_impl)); }

	virtual int first_file() noexcept override { return m_impl->first_file(); }
	virtual int next_file() noexcept override { return m_impl->next_file(); }

	virtual int search(std::uint32_t crc) noexcept override
	{
		return m_impl->search(crc);
	}
	virtual int search(std::string_view filename, bool partialpath) noexcept override
	{
		return m_impl->search(filename, partialpath);
	}
	virtual int search(std::uint32_t crc, std::string_view filename, bool partialpath) noexcept override
	{
		return m_impl->search(crc, filename, partialpath);
	}

	virtual bool current_is_directory() const noexcept override { return m_impl->current_is_directory(); }
	virtual const std::string &current_name() const noexcept override { return m_impl->current_name(); }
	virtual std::uint64_t current_uncompressed_length() const noexcept override { return m_impl->current_uncompressed_length(); }
	virtual std::chrono::system_clock::time_point current_last_modified() const noexcept override { return m_impl->current_last_modified(); }
	virtual std::uint32_t current_crc() const noexcept override { return m_impl->current_crc(); }

	virtual std::error_condition decompress(void *buffer, std::size_t length) noexcept override { return m_impl->decompress(buffer, length); }

private:
	m7z_file_impl::ptr m_impl;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

std::array<m7z_file_impl::ptr, m7z_file_impl::CACHE_SIZE> m7z_file_impl::s_cache;
std::mutex m7z_file_impl::s_cache_mutex;



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

m7z_file_impl::m7z_file_impl(std::string &&filename) noexcept
	: m_filename(std::move(filename))
	, m_curr_file_idx(-1)
	, m_curr_is_dir(false)
	, m_curr_name()
	, m_curr_length(0)
	, m_curr_modified()
	, m_curr_crc(0)
	, m_utf16_buf()
	, m_uchar_buf()
	, m_utf8_buf()
	, m_inited(false)
	, m_block_index(0)
	, m_out_buffer(nullptr)
	, m_out_buffer_size(0)
{
	m_alloc_imp.Alloc = &SzAlloc;
	m_alloc_imp.Free = &SzFree;

	m_alloc_temp_imp.Alloc = &SzAllocTemp;
	m_alloc_temp_imp.Free = &SzFreeTemp;

	LookToRead2_CreateVTable(&m_look_stream, False);
	m_look_stream.realStream = &m_archive_stream;
	m_look_stream.buf = m_look_stream_buf;
	m_look_stream.bufSize = std::size(m_look_stream_buf);
	LookToRead2_INIT(&m_look_stream);
}


std::error_condition m7z_file_impl::initialize() noexcept
{
	try
	{
		if (m_utf16_buf.size() < 128)
			m_utf16_buf.resize(128);
		if (m_uchar_buf.size() < 128)
			m_uchar_buf.resize(128);
		m_utf8_buf.reserve(512);
	}
	catch (...)
	{
		return std::errc::not_enough_memory;
	}

	if (!m_archive_stream.file)
	{
		osd_file::ptr file;
		std::error_condition const err = osd_file::open(m_filename, OPEN_FLAG_READ, file, m_archive_stream.length);
		if (err)
			return err;
		m_archive_stream.file = osd_file_read(std::move(file));
		osd_printf_verbose("un7z: opened archive file %s\n", m_filename);
	}
	else if (!m_archive_stream.length)
	{
		std::error_condition const err = m_archive_stream.file->length(m_archive_stream.length);
		if (err)
		{
			osd_printf_verbose(
					"un7z: error getting length of archive file %s (%s:%d %s)\n",
					m_filename, err.category().name(), err.value(), err.message());
			return err;
		}
	}

	// TODO: coordinate this with other LZMA users in the codebase?
	struct crc_table_generator { crc_table_generator() { CrcGenerateTable(); } };
	static crc_table_generator crc_table;

	SzArEx_Init(&m_db);
	m_inited = true;
	SRes const res = SzArEx_Open(&m_db, &m_look_stream.vt, &m_alloc_imp, &m_alloc_temp_imp);
	if (res != SZ_OK)
	{
		osd_printf_error("un7z: error opening %s as 7z archive (%d)\n", m_filename, int(res));
		switch (res)
		{
		case SZ_ERROR_UNSUPPORTED:  return archive_file::error::UNSUPPORTED;
		case SZ_ERROR_MEM:          return std::errc::not_enough_memory;
		case SZ_ERROR_INPUT_EOF:    return archive_file::error::FILE_TRUNCATED;
		default:                    return std::errc::io_error; // TODO: better default error?
		}
	}

	return std::error_condition();
}


/*-------------------------------------------------
    _7z_file_close - close a _7Z file and add it
    to the cache
-------------------------------------------------*/

void m7z_file_impl::close(ptr &&archive) noexcept
{
	// if the filename isn't empty, the implementation can be cached
	if (archive && !archive->m_filename.empty())
	{
		// close the open files
		osd_printf_verbose("un7z: closing archive file %s and sending to cache\n", archive->m_filename);
		archive->m_archive_stream.file.reset();

		// find the first nullptr entry in the cache
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		std::size_t cachenum;
		for (cachenum = 0; cachenum < s_cache.size(); cachenum++)
			if (!s_cache[cachenum])
				break;

		// if no room left in the cache, free the bottommost entry
		if (cachenum == s_cache.size())
		{
			cachenum--;
			osd_printf_verbose("un7z: removing %s from cache to make space\n", s_cache[cachenum]->m_filename);
			s_cache[cachenum].reset();
		}

		// move everyone else down and place us at the top
		for ( ; cachenum > 0; cachenum--)
			s_cache[cachenum] = std::move(s_cache[cachenum - 1]);
		s_cache[0] = std::move(archive);
	}

	// make sure it's cleaned up
	archive.reset();
}



/***************************************************************************
    7Z FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    _7z_file_decompress - decompress a file
    from a _7Z into the target buffer
-------------------------------------------------*/

std::error_condition m7z_file_impl::decompress(void *buffer, std::size_t length) noexcept
{
	// if we don't have enough buffer, error
	if (length < m_curr_length)
	{
		osd_printf_error("un7z: buffer too small to decompress %s from %s\n", m_curr_name, m_filename);
		return archive_file::error::BUFFER_TOO_SMALL;
	}

	// make sure the file is open..
	if (!m_archive_stream.file)
	{
		m_archive_stream.currfpos = 0; // FIXME: should it really be changing the file pointer out from under LZMA?
		osd_file::ptr file;
		std::error_condition const err = osd_file::open(m_filename, OPEN_FLAG_READ, file, m_archive_stream.length);
		if (err)
		{
			osd_printf_error(
					"un7z: error reopening archive file %s (%s:%d %s)\n",
					m_filename, err.category().name(), err.value(), err.message());
			return err;
		}
		m_archive_stream.file = osd_file_read(std::move(file));
		osd_printf_verbose("un7z: reopened archive file %s\n", m_filename);
	}

	std::size_t offset(0);
	std::size_t out_size_processed(0);
	SRes const res = SzArEx_Extract(
			&m_db, &m_look_stream.vt, m_curr_file_idx,          // requested file
			&m_block_index, &m_out_buffer, &m_out_buffer_size,  // solid block caching
			&offset, &out_size_processed,                       // data size/offset
			&m_alloc_imp, &m_alloc_temp_imp);                   // allocator helpers
	if (res != SZ_OK)
	{
		osd_printf_error("un7z: error decompressing %s from %s (%d)\n", m_curr_name, m_filename, int(res));
		switch (res)
		{
		case SZ_ERROR_UNSUPPORTED:  return archive_file::error::UNSUPPORTED;
		case SZ_ERROR_MEM:          return std::errc::not_enough_memory;
		case SZ_ERROR_INPUT_EOF:    return archive_file::error::FILE_TRUNCATED;
		default:                    return archive_file::error::DECOMPRESS_ERROR;
		}
	}

	// copy to destination buffer
	std::memcpy(buffer, m_out_buffer + offset, (std::min<std::size_t>)(length, out_size_processed));
	return std::error_condition();
}


int m7z_file_impl::search(
		int i,
		std::uint32_t search_crc,
		std::string_view search_filename,
		bool matchcrc,
		bool matchname,
		bool partialpath) noexcept
{
	try
	{
		for ( ; i < m_db.NumFiles; i++)
		{
			make_utf8_name(i);
			bool const is_dir(SzArEx_IsDir(&m_db, i));
			const std::uint64_t size(SzArEx_GetFileSize(&m_db, i));
			const std::uint32_t crc(m_db.CRCs.Vals[i]);

			const bool crcmatch(SzBitArray_Check(m_db.CRCs.Defs, i) && (crc == search_crc));
			bool found;
			if (!matchname)
			{
				found = !matchcrc || (crcmatch && !is_dir);
			}
			else
			{
				auto const partialoffset = m_utf8_buf.size() - search_filename.length();
				const bool namematch =
						(search_filename.length() == m_utf8_buf.size()) &&
						(search_filename.empty() || !core_strnicmp(&search_filename[0], &m_utf8_buf[0], search_filename.length()));
				bool const partialmatch =
						partialpath &&
						((m_utf8_buf.size() > search_filename.length()) && (m_utf8_buf[partialoffset - 1] == '/')) &&
						(search_filename.empty() || !core_strnicmp(&search_filename[0], &m_utf8_buf[partialoffset], search_filename.length()));
				found = (!matchcrc || crcmatch) && (namematch || partialmatch);
			}

			if (found)
			{
				// set the name first - resizing it can throw an exception, and we want the state to be consistent
				m_curr_name.assign(m_utf8_buf.begin(), m_utf8_buf.end());
				m_curr_file_idx = i;
				m_curr_is_dir = is_dir;
				m_curr_length = size;
				set_curr_modified();
				m_curr_crc = crc;

				return i;
			}
		}
	}
	catch (...)
	{
		// allocation error handling name
	}
	return -1;
}


void m7z_file_impl::make_utf8_name(int index)
{
	std::size_t len, out_pos;

	len = SzArEx_GetFileNameUtf16(&m_db, index, nullptr);
	m_utf16_buf.resize(std::max<std::size_t>(m_utf16_buf.size(), len));
	SzArEx_GetFileNameUtf16(&m_db, index, &m_utf16_buf[0]);

	m_uchar_buf.resize(std::max<std::size_t>(m_uchar_buf.size(), len));
	out_pos = 0;
	for (std::size_t in_pos = 0; in_pos < (len - 1); )
	{
		const int used = uchar_from_utf16(&m_uchar_buf[out_pos], reinterpret_cast<char16_t const *>(&m_utf16_buf[in_pos]), len - in_pos);
		if (used < 0)
		{
			in_pos++;
			m_uchar_buf[out_pos++] = 0x00fffd; // Unicode REPLACEMENT CHARACTER
		}
		else
		{
			assert(used > 0);
			in_pos += used;
			out_pos++;
		}
	}
	len = out_pos;

	m_utf8_buf.resize((std::max<std::size_t>)(m_utf8_buf.size(), 4 * len));
	out_pos = 0;
	for (std::size_t in_pos = 0; in_pos < len; in_pos++)
	{
		int produced = utf8_from_uchar(&m_utf8_buf[out_pos], m_utf8_buf.size() - out_pos, m_uchar_buf[in_pos]);
		if (produced < 0)
			produced = utf8_from_uchar(&m_utf8_buf[out_pos], m_utf8_buf.size() - out_pos, 0x00fffd);
		if (produced >= 0)
			out_pos += produced;
		assert(out_pos < m_utf8_buf.size());
	}
	m_utf8_buf.resize(out_pos);
}


void m7z_file_impl::set_curr_modified() noexcept
{
	if (SzBitWithVals_Check(&m_db.MTime, m_curr_file_idx))
	{
		CNtfsFileTime const &file_time(m_db.MTime.Vals[m_curr_file_idx]);
		try
		{
			auto ticks = ntfs_duration_from_filetime(file_time.High, file_time.Low);
			m_curr_modified = system_clock_time_point_from_ntfs_duration(ticks);
			return;
		}
		catch (...)
		{
		}
	}

	// no modification time available, or out-of-range exception
	m_curr_modified = std::chrono::system_clock::from_time_t(std::time_t(0));
}


} // anonymous namespace


std::error_condition archive_file::open_7z(std::string_view filename, ptr &result) noexcept
{
	// ensure we start with a nullptr result
	result.reset();

	// see if we are in the cache, and reopen if so
	m7z_file_impl::ptr newimpl(m7z_file_impl::find_cached(filename));

	if (!newimpl)
	{
		// allocate memory for the 7z file structure
		try { newimpl = std::make_unique<m7z_file_impl>(std::string(filename)); }
		catch (...) { return std::errc::not_enough_memory; }
		auto const err = newimpl->initialize();
		if (err)
			return err;
	}

	// allocate the archive API wrapper
	result.reset(new (std::nothrow) m7z_file_wrapper(std::move(newimpl)));
	if (result)
	{
		return std::error_condition();
	}
	else
	{
		m7z_file_impl::close(std::move(newimpl));
		return std::errc::not_enough_memory;
	}
}

std::error_condition archive_file::open_7z(random_read::ptr &&file, ptr &result) noexcept
{
	// ensure we start with a nullptr result
	result.reset();

	// allocate memory for the zip_file structure
	m7z_file_impl::ptr newimpl(new (std::nothrow) m7z_file_impl(std::move(file)));
	if (!newimpl)
		return std::errc::not_enough_memory;
	auto const err = newimpl->initialize();
	if (err)
		return err;

	// allocate the archive API wrapper
	result.reset(new (std::nothrow) m7z_file_wrapper(std::move(newimpl)));
	if (result)
	{
		return std::error_condition();
	}
	else
	{
		m7z_file_impl::close(std::move(newimpl));
		return std::errc::not_enough_memory;
	}
}


/*-------------------------------------------------
    _7z_file_cache_clear - clear the _7Z file
    cache and free all memory
-------------------------------------------------*/

void m7z_file_cache_clear() noexcept
{
	// This is a trampoline called from unzip.cpp to avoid the need to have the zip and 7zip code in one file
	m7z_file_impl::cache_clear();
}

} // namespace util
