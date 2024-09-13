// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.cpp

    Functions to manipulate data within ZIP files.

***************************************************************************/

#include "unzip.h"

#include "corestr.h"
#include "hashing.h"
#include "ioprocs.h"
#include "multibyte.h"
#include "timeconv.h"

#include "osdcore.h"
#include "osdfile.h"

#include "lzma/C/LzmaDec.h"

#include <zlib.h>
#include <zstd.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <optional>
#include <ratio>
#include <utility>
#include <vector>


namespace util {

namespace {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class archive_category_impl : public std::error_category
{
public:
	virtual char const *name() const noexcept override { return "archive"; }

	virtual std::string message(int condition) const override
	{
		using namespace std::literals;
		static std::string_view const s_messages[] = {
				"No error"sv,
				"Bad archive signature"sv,
				"Decompression error"sv,
				"Archive file truncated"sv,
				"Archive file corrupt"sv,
				"Archive file uses unsupported features"sv,
				"Buffer too small for data"sv };
		if ((0 <= condition) && (std::size(s_messages) > condition))
			return std::string(s_messages[condition]);
		else
			return "Unknown error"s;
	}
};


class zip_file_impl
{
public:
	using ptr = std::unique_ptr<zip_file_impl>;

	zip_file_impl(std::string &&filename) noexcept
		: m_filename(std::move(filename))
	{
		std::fill(m_buffer.begin(), m_buffer.end(), 0);
	}

	zip_file_impl(random_read::ptr &&file) noexcept
		: zip_file_impl(std::string())
	{
		m_file = std::move(file);
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
				osd_printf_verbose("unzip: found %s in cache\n", filename);
				return result;
			}
		}
		return ptr();
	}

	static void close(ptr &&zip) noexcept;

	static void cache_clear() noexcept
	{
		// clear call cache entries
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		for (auto &cached : s_cache)
			cached.reset();
	}

	std::error_condition initialize() noexcept
	{
		// read ecd data
		auto const ziperr = read_ecd();
		if (ziperr)
			return ziperr;

		// verify that we can work with this zipfile (no disk spanning allowed)
		if (m_ecd.disk_number != m_ecd.cd_start_disk_number)
		{
			osd_printf_error("unzip: %s central directory starts in another segment\n", m_filename);
			return archive_file::error::UNSUPPORTED;
		}
		if (m_ecd.cd_disk_entries != m_ecd.cd_total_entries)
		{
			osd_printf_error("unzip: %s not all central directory entries reside in this segment\n", m_filename);
			return archive_file::error::UNSUPPORTED;
		}
		if (std::size_t(m_ecd.cd_size) != m_ecd.cd_size)
		{
			osd_printf_error("unzip: %s central directory too large to load\n", m_filename);
			return archive_file::error::UNSUPPORTED;
		}

		// allocate memory for the central directory
		try { m_cd.resize(std::size_t(m_ecd.cd_size)); }
		catch (...)
		{
			osd_printf_error("unzip: %s failed to allocate memory for central directory\n", m_filename);
			return std::errc::not_enough_memory;
		}

		// read the central directory
		auto cd_remaining(m_ecd.cd_size);
		std::size_t cd_offs(0);
		while (cd_remaining)
		{
			std::size_t const chunk(std::size_t(std::min<std::uint64_t>(std::numeric_limits<std::size_t>::max(), cd_remaining)));
			auto const [filerr, read_length] = read_at(*m_file, m_ecd.cd_start_disk_offset + cd_offs, &m_cd[cd_offs], chunk);
			if (filerr)
			{
				osd_printf_error(
						"unzip: %s error reading central directory (%s:%d %s)\n",
						m_filename, filerr.category().name(), filerr.value(), filerr.message());
				return filerr;
			}
			if (!read_length)
			{
				osd_printf_error("unzip: %s unexpectedly reached end-of-file while reading central directory\n", m_filename);
				return archive_file::error::FILE_TRUNCATED;
			}
			cd_remaining -= read_length;
			cd_offs += read_length;
		}
		osd_printf_verbose("unzip: read %s central directory\n", m_filename);

		return std::error_condition();
	}

	int first_file() noexcept
	{
		m_cd_pos = 0;
		return search(0, std::string_view(), false, false, false);
	}

	int next_file() noexcept
	{
		return search(0, std::string_view(), false, false, false);
	}

	int search(std::uint32_t crc) noexcept
	{
		m_cd_pos = 0;
		return search(crc, std::string_view(), true, false, false);
	}

	int search(std::string_view filename, bool partialpath) noexcept
	{
		m_cd_pos = 0;
		return search(0, filename, false, true, partialpath);
	}

	int search(std::uint32_t crc, std::string_view filename, bool partialpath) noexcept
	{
		m_cd_pos = 0;
		return search(crc, filename, true, true, partialpath);
	}

	bool current_is_directory() const noexcept { return m_curr_is_dir; }

	const std::string &current_name() const noexcept { return m_header.file_name; }

	std::uint64_t current_uncompressed_length() const noexcept { return m_header.uncompressed_length; }

	std::chrono::system_clock::time_point current_last_modified() const noexcept
	{
		if (!m_header.modified)
			m_header.modified = decode_dos_time(m_header.modified_date, m_header.modified_time);
		return *m_header.modified;
	}

	std::uint32_t current_crc() const noexcept { return m_header.crc; }

	std::error_condition decompress(void *buffer, std::size_t length) noexcept;

private:
	zip_file_impl(const zip_file_impl &) = delete;
	zip_file_impl(zip_file_impl &&) = delete;
	zip_file_impl &operator=(const zip_file_impl &) = delete;
	zip_file_impl &operator=(zip_file_impl &&) = delete;

	int search(std::uint32_t search_crc, std::string_view search_filename, bool matchcrc, bool matchname, bool partialpath) noexcept;

	std::error_condition reopen() noexcept
	{
		if (!m_file)
		{
			osd_file::ptr file;
			auto const filerr = osd_file::open(m_filename, OPEN_FLAG_READ, file, m_length);
			if (filerr)
			{
				// this would spam every time it looks for a non-existent archive, which is a lot
				//osd_printf_error("unzip: error reopening archive file %s (%s:%d %s)\n", m_filename, filerr.category().name(), filerr.value(), filerr.message());
				return filerr;
			}
			m_file = osd_file_read(std::move(file));
			if (!m_file)
			{
				osd_printf_error("unzip: not enough memory to open archive file %s\n", m_filename);
				return std::errc::not_enough_memory;
			}
			osd_printf_verbose("unzip: opened archive file %s\n", m_filename);
		}
		else if (!m_length)
		{
			auto const filerr = m_file->length(m_length);
			if (filerr)
			{
				osd_printf_verbose(
						"unzip: error getting length of archive file %s (%s:%d %s)\n",
						m_filename, filerr.category().name(), filerr.value(), filerr.message());
				return filerr;
			}
		}
		return std::error_condition();
	}

	static std::chrono::system_clock::time_point decode_dos_time(std::uint16_t date, std::uint16_t time) noexcept
	{
		// FIXME: work out why this doesn't always work
		// negative tm_isdst should automatically determine whether DST is in effect for the date,
		// but on Windows apparently it doesn't, so you get time offsets
		std::tm datetime;
		datetime.tm_sec = (time << 1) & 0x003e;
		datetime.tm_min = (time >> 5) & 0x003f;
		datetime.tm_hour = (time >> 11) & 0x001f;
		datetime.tm_mday = (date >> 0) & 0x001f;
		datetime.tm_mon = ((date >> 5) & 0x000f) - 1;
		datetime.tm_year = ((date >> 9) & 0x007f) + 80;
		datetime.tm_wday = 0;
		datetime.tm_yday = 0;
		datetime.tm_isdst = -1;
		return std::chrono::system_clock::from_time_t(std::mktime(&datetime));
	}

	// ZIP file parsing
	std::error_condition read_ecd() noexcept;
	std::error_condition get_compressed_data_offset(std::uint64_t &offset) noexcept;

	// decompression interfaces
	std::error_condition decompress_data_type_0(std::uint64_t offset, void *buffer, std::size_t length) noexcept;
	std::error_condition decompress_data_type_8(std::uint64_t offset, void *buffer, std::size_t length) noexcept;
	std::error_condition decompress_data_type_14(std::uint64_t offset, void *buffer, std::size_t length) noexcept;
	std::error_condition decompress_data_type_93(std::uint64_t offset, void *buffer, std::size_t length) noexcept;

	struct file_header
	{
	private:
		using optional_time_point = std::optional<std::chrono::system_clock::time_point>;

	public:
		file_header() noexcept { }
		file_header(file_header const &) = default;
		file_header(file_header &&) noexcept = default;
		file_header &operator=(file_header const &) = default;
		file_header &operator=(file_header &&) noexcept = default;

		std::uint16_t               version_created;        // version made by
		std::uint16_t               version_needed;         // version needed to extract
		std::uint16_t               bit_flag;               // general purpose bit flag
		std::uint16_t               compression;            // compression method
		mutable optional_time_point modified;               // last mod file date/time
		std::uint32_t               crc;                    // crc-32
		std::uint64_t               compressed_length;      // compressed size
		std::uint64_t               uncompressed_length;    // uncompressed size
		std::uint32_t               start_disk_number;      // disk number start
		std::uint64_t               local_header_offset;    // relative offset of local header
		std::string                 file_name;              // file name

		std::uint16_t               modified_date, modified_time;
	};

	// contains extracted end of central directory information
	struct ecd
	{
		std::uint32_t   disk_number;            // number of this disk
		std::uint32_t   cd_start_disk_number;   // number of the disk with the start of the central directory
		std::uint64_t   cd_disk_entries;        // total number of entries in the central directory on this disk
		std::uint64_t   cd_total_entries;       // total number of entries in the central directory
		std::uint64_t   cd_size;                // size of the central directory
		std::uint64_t   cd_start_disk_offset;   // offset of start of central directory with respect to the starting disk number
	};

	static constexpr std::size_t        DECOMPRESS_BUFSIZE = 16384;
	static constexpr std::size_t        CACHE_SIZE = 8; // number of open files to cache
	static std::array<ptr, CACHE_SIZE>  s_cache;
	static std::mutex                   s_cache_mutex;

	const std::string           m_filename;                 // copy of ZIP filename (for caching)
	random_read::ptr            m_file;                     // file handle
	std::uint64_t               m_length = 0;               // length of zip file

	ecd                         m_ecd;                      // end of central directory

	std::vector<std::uint8_t>   m_cd;                       // central directory raw data
	std::uint32_t               m_cd_pos = 0;               // position in central directory
	file_header                 m_header;                   // current file header
	bool                        m_curr_is_dir = false;      // current file is directory

	std::array<std::uint8_t, DECOMPRESS_BUFSIZE> m_buffer;  // buffer for decompression
};


class zip_file_wrapper : public archive_file
{
public:
	zip_file_wrapper(zip_file_impl::ptr &&impl) noexcept : m_impl(std::move(impl)) { assert(m_impl); }
	virtual ~zip_file_wrapper() { zip_file_impl::close(std::move(m_impl)); }

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
	zip_file_impl::ptr m_impl;
};


class reader_base
{
protected:
	reader_base(void const *buf) noexcept : m_buffer(reinterpret_cast<std::uint8_t const *>(buf)) { }

	std::uint8_t read_byte(std::size_t offs) const noexcept
	{
		return m_buffer[offs];
	}
	std::uint16_t read_word(std::size_t offs) const noexcept
	{
		return get_u16le(&m_buffer[offs]);
	}
	std::uint32_t read_dword(std::size_t offs) const noexcept
	{
		return get_u32le(&m_buffer[offs]);
	}
	std::uint64_t read_qword(std::size_t offs) const noexcept
	{
		return get_u64le(&m_buffer[offs]);
	}
	std::string read_string(std::size_t offs, std::string::size_type len) const
	{
		return std::string(reinterpret_cast<char const *>(m_buffer + offs), len);
	}
	void read_string(std::string &result, std::size_t offs, std::string::size_type len) const
	{
		result.assign(reinterpret_cast<char const *>(m_buffer + offs), len);
	}

	std::uint8_t const *m_buffer;
};


class extra_field_reader : private reader_base
{
public:
	extra_field_reader(void const *buf, std::size_t len) noexcept : reader_base(buf), m_length(len) { }

	std::uint16_t         header_id() const noexcept    { return read_word(0x00); }
	std::uint16_t         data_size() const noexcept    { return read_word(0x02); }
	void const *          data() const noexcept         { return m_buffer + 0x04; }
	extra_field_reader    next() const noexcept         { return extra_field_reader(m_buffer + total_length(), m_length - total_length()); }

	bool length_sufficient() const noexcept { return (m_length >= minimum_length()) && (m_length >= total_length()); }

	std::size_t total_length() const noexcept { return minimum_length() + data_size(); }
	static constexpr std::size_t minimum_length() { return 0x04; }

private:
	std::size_t m_length;
};


class local_file_header_reader : private reader_base
{
public:
	local_file_header_reader(void const *buf) noexcept : reader_base(buf) { }

	std::uint32_t       signature() const noexcept              { return read_dword(0x00); }
	std::uint8_t        version_needed() const noexcept         { return m_buffer[0x04]; }
	std::uint8_t        os_needed() const noexcept              { return m_buffer[0x05]; }
	std::uint16_t       general_flag() const noexcept           { return read_word(0x06); }
	std::uint16_t       compression_method() const noexcept     { return read_word(0x08); }
	std::uint16_t       modified_time() const noexcept          { return read_word(0x0a); }
	std::uint16_t       modified_date() const noexcept          { return read_word(0x0c); }
	std::uint32_t       crc32() const noexcept                  { return read_dword(0x0e); }
	std::uint32_t       compressed_size() const noexcept        { return read_dword(0x12); }
	std::uint32_t       uncompressed_size() const noexcept      { return read_dword(0x16); }
	std::uint16_t       file_name_length() const noexcept       { return read_word(0x1a); }
	std::uint16_t       extra_field_length() const noexcept     { return read_word(0x1c); }
	std::string         file_name() const                       { return read_string(0x1e, file_name_length()); }
	void                file_name(std::string &result) const    { read_string(result, 0x1e, file_name_length()); }
	extra_field_reader  extra_field() const noexcept            { return extra_field_reader(m_buffer + 0x1e + file_name_length(), extra_field_length()); }

	bool                signature_correct() const noexcept      { return signature() == 0x04034b50; }

	std::size_t total_length() const noexcept { return minimum_length() + file_name_length() + extra_field_length(); }
	static constexpr std::size_t minimum_length() { return 0x1e; }
};


class central_dir_entry_reader : private reader_base
{
public:
	central_dir_entry_reader(void const *buf) noexcept : reader_base(buf) { }

	std::uint32_t       signature() const noexcept              { return read_dword(0x00); }
	std::uint8_t        version_created() const noexcept        { return m_buffer[0x04]; }
	std::uint8_t        os_created() const noexcept             { return m_buffer[0x05]; }
	std::uint8_t        version_needed() const noexcept         { return m_buffer[0x06]; }
	std::uint8_t        os_needed() const noexcept              { return m_buffer[0x07]; }
	std::uint16_t       general_flag() const noexcept           { return read_word(0x08); }
	std::uint16_t       compression_method() const noexcept     { return read_word(0x0a); }
	std::uint16_t       modified_time() const noexcept          { return read_word(0x0c); }
	std::uint16_t       modified_date() const noexcept          { return read_word(0x0e); }
	std::uint32_t       crc32() const noexcept                  { return read_dword(0x10); }
	std::uint32_t       compressed_size() const noexcept        { return read_dword(0x14); }
	std::uint32_t       uncompressed_size() const noexcept      { return read_dword(0x18); }
	std::uint16_t       file_name_length() const noexcept       { return read_word(0x1c); }
	std::uint16_t       extra_field_length() const noexcept     { return read_word(0x1e); }
	std::uint16_t       file_comment_length() const noexcept    { return read_word(0x20); }
	std::uint16_t       start_disk() const noexcept             { return read_word(0x22); }
	std::uint16_t       int_file_attr() const noexcept          { return read_word(0x24); }
	std::uint32_t       ext_file_attr() const noexcept          { return read_dword(0x26); }
	std::uint32_t       header_offset() const noexcept          { return read_dword(0x2a); }
	std::string         file_name() const                       { return read_string(0x2e, file_name_length()); }
	void                file_name(std::string &result) const    { read_string(result, 0x2e, file_name_length()); }
	extra_field_reader  extra_field() const noexcept            { return extra_field_reader(m_buffer + 0x2e + file_name_length(), extra_field_length()); }
	std::string         file_comment() const                    { return read_string(0x2e + file_name_length() + extra_field_length(), file_comment_length()); }
	void                file_comment(std::string &result) const { read_string(result, 0x2e + file_name_length() + extra_field_length(), file_comment_length()); }

	bool                signature_correct() const noexcept      { return signature() == 0x02014b50; }

	std::size_t total_length() const noexcept { return minimum_length() + file_name_length() + extra_field_length() + file_comment_length(); }
	static constexpr std::size_t minimum_length() { return 0x2e; }
};


class ecd64_reader : private reader_base
{
public:
	ecd64_reader(void const *buf) noexcept : reader_base(buf) { }

	std::uint32_t   signature() const noexcept          { return read_dword(0x00); }
	std::uint64_t   ecd64_size() const noexcept         { return read_qword(0x04); }
	std::uint8_t    version_created() const noexcept    { return m_buffer[0x0c]; }
	std::uint8_t    os_created() const noexcept         { return m_buffer[0x0d]; }
	std::uint8_t    version_needed() const noexcept     { return m_buffer[0x0e]; }
	std::uint8_t    os_needed() const noexcept          { return m_buffer[0x0f]; }
	std::uint32_t   this_disk_no() const noexcept       { return read_dword(0x10); }
	std::uint32_t   dir_start_disk() const noexcept     { return read_dword(0x14); }
	std::uint64_t   dir_disk_entries() const noexcept   { return read_qword(0x18); }
	std::uint64_t   dir_total_entries() const noexcept  { return read_qword(0x20); }
	std::uint64_t   dir_size() const noexcept           { return read_qword(0x28); }
	std::uint64_t   dir_offset() const noexcept         { return read_qword(0x30); }
	void const *    extensible_data() const noexcept    { return m_buffer + 0x38; }

	bool            signature_correct() const noexcept  { return signature() == 0x06064b50; }

	std::size_t total_length() const noexcept { return 0x0c + ecd64_size(); }
	static constexpr std::size_t minimum_length() { return 0x38; }
};


class ecd64_locator_reader : private reader_base
{
public:
	ecd64_locator_reader(void const *buf) noexcept : reader_base(buf) { }

	std::uint32_t   signature() const noexcept          { return read_dword(0x00); }
	std::uint32_t   ecd64_disk() const noexcept         { return read_dword(0x04); }
	std::uint64_t   ecd64_offset() const noexcept       { return read_qword(0x08); }
	std::uint32_t   total_disks() const noexcept        { return read_dword(0x10); }

	bool            signature_correct() const noexcept  { return signature() == 0x07064b50; }

	std::size_t total_length() const noexcept { return minimum_length(); }
	static constexpr std::size_t minimum_length() { return 0x14; }
};


class ecd_reader : private reader_base
{
public:
	ecd_reader(void const *buf) noexcept : reader_base(buf) { }

	std::uint32_t   signature() const noexcept          { return read_dword(0x00); }
	std::uint16_t   this_disk_no() const noexcept       { return read_word(0x04); }
	std::uint16_t   dir_start_disk() const noexcept     { return read_word(0x06); }
	std::uint16_t   dir_disk_entries() const noexcept   { return read_word(0x08); }
	std::uint16_t   dir_total_entries() const noexcept  { return read_word(0x0a); }
	std::uint32_t   dir_size() const noexcept           { return read_dword(0x0c); }
	std::uint32_t   dir_offset() const noexcept         { return read_dword(0x10); }
	std::uint16_t   comment_length() const noexcept     { return read_word(0x14); }
	std::string     comment() const                     { return read_string(0x16, comment_length()); }
	void            comment(std::string &result) const  { read_string(result, 0x16, comment_length()); }

	bool            signature_correct() const noexcept  { return signature() == 0x06054b50; }

	std::size_t total_length() const noexcept { return minimum_length() + comment_length(); }
	static constexpr std::size_t minimum_length() { return 0x16; }
};


class zip64_ext_info_reader : private reader_base
{
public:
	template <typename T>
	zip64_ext_info_reader(
			T const &header,
			extra_field_reader const &field) noexcept
		: reader_base(field.data())
		, m_uncompressed_size(header.uncompressed_size())
		, m_compressed_size(header.compressed_size())
		, m_header_offset(header.header_offset())
		, m_start_disk(header.start_disk())
		, m_offs_compressed_size((0xffff'ffffU != m_uncompressed_size) ? 0 : 8)
		, m_offs_header_offset(m_offs_compressed_size + ((0xffff'ffffU != m_compressed_size) ? 0 : 8))
		, m_offs_start_disk(m_offs_header_offset + ((0xffff'ffffU != m_header_offset) ? 0 : 8))
		, m_offs_end(m_offs_start_disk + ((0xffffU != m_start_disk) ? 0 : 4))
	{
	}

	std::uint64_t   uncompressed_size() const noexcept  { return (0xffff'ffffU != m_uncompressed_size) ? m_uncompressed_size : read_qword(0x00); }
	std::uint64_t   compressed_size() const noexcept    { return (0xffff'ffffU != m_compressed_size) ? m_compressed_size : read_qword(m_offs_compressed_size); }
	std::uint64_t   header_offset() const noexcept      { return (0xffff'ffffU != m_header_offset) ? m_header_offset : read_qword(m_offs_header_offset); }
	std::uint32_t   start_disk() const noexcept         { return (0xffffU != m_start_disk) ? m_start_disk : read_dword(m_offs_start_disk); }

	std::size_t total_length() const noexcept { return minimum_length() + m_offs_end; }
	static constexpr std::size_t minimum_length() { return 0x00; }

private:
	std::uint32_t   m_uncompressed_size;
	std::uint32_t   m_compressed_size;
	std::uint32_t   m_header_offset;
	std::uint16_t   m_start_disk;

	std::size_t     m_offs_compressed_size;
	std::size_t     m_offs_header_offset;
	std::size_t     m_offs_start_disk;
	std::size_t     m_offs_end;
};


class utf8_path_reader : private reader_base
{
public:
	utf8_path_reader(extra_field_reader const &field) noexcept : reader_base(field.data()), m_length(field.data_size()) { }

	std::uint8_t    version() const noexcept                { return m_buffer[0]; }
	std::uint32_t   name_crc32() const noexcept             { return read_dword(0x01); }
	std::string     unicode_name() const                    { return read_string(0x05, m_length - 0x05); }
	void            unicode_name(std::string &result) const { return read_string(result, 0x05, m_length - 0x05); }

	std::size_t total_length() const noexcept { return m_length; }
	static constexpr std::size_t minimum_length() { return 0x05; }

private:
	std::size_t m_length;
};


class ntfs_tag_reader : private reader_base
{
public:
	ntfs_tag_reader(void const *buf, std::size_t len) noexcept : reader_base(buf), m_length(len) { }

	std::uint16_t     tag() const noexcept  { return read_word(0x00); }
	std::uint16_t     size() const noexcept { return read_word(0x02); }
	void const *      data() const noexcept { return m_buffer + 0x04; }
	ntfs_tag_reader   next() const noexcept { return ntfs_tag_reader(m_buffer + total_length(), m_length - total_length()); }

	bool length_sufficient() const noexcept { return (m_length >= minimum_length()) && (m_length >= total_length()); }

	std::size_t total_length() const noexcept { return minimum_length() + size(); }
	static constexpr std::size_t minimum_length() { return 0x04; }

private:
	std::size_t m_length;
};


class ntfs_reader : private reader_base
{
public:
	ntfs_reader(extra_field_reader const &field) noexcept : reader_base(field.data()), m_length(field.data_size()) { }

	std::uint32_t   reserved() const noexcept   { return read_dword(0x00); }
	ntfs_tag_reader tag1() const noexcept       { return ntfs_tag_reader(m_buffer + 0x04, m_length - 4); }

	std::size_t total_length() const noexcept { return m_length; }
	static constexpr std::size_t minimum_length() { return 0x08; }

private:
	std::size_t m_length;
};


class ntfs_times_reader : private reader_base
{
public:
	ntfs_times_reader(ntfs_tag_reader const &tag) noexcept : reader_base(tag.data()) { }

	std::uint64_t   mtime() const noexcept  { return read_qword(0x00); }
	std::uint64_t   atime() const noexcept  { return read_qword(0x08); }
	std::uint64_t   ctime() const noexcept  { return read_qword(0x10); }

	std::size_t total_length() const noexcept { return minimum_length(); }
	static constexpr std::size_t minimum_length() { return 0x18; }
};


class general_flag_reader
{
public:
	general_flag_reader(std::uint16_t val) : m_value(val) { }

	bool        encrypted() const noexcept              { return bool(m_value & 0x0001); }
	bool        implode_8k_dict() const noexcept        { return bool(m_value & 0x0002); }
	bool        implode_3_trees() const noexcept        { return bool(m_value & 0x0004); }
	unsigned    deflate_option() const noexcept         { return unsigned((m_value >> 1) & 0x0003); }
	bool        lzma_eos_mark() const noexcept          { return bool(m_value & 0x0002); }
	bool        use_descriptor() const noexcept         { return bool(m_value & 0x0008); }
	bool        patch_data() const noexcept             { return bool(m_value & 0x0020); }
	bool        strong_encryption() const noexcept      { return bool(m_value & 0x0040); }
	bool        utf8_encoding() const noexcept          { return bool(m_value & 0x0800); }
	bool        directory_encryption() const noexcept   { return bool(m_value & 0x2000); }

private:
	std::uint16_t m_value;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

archive_category_impl const f_archive_category_instance;

std::array<zip_file_impl::ptr, zip_file_impl::CACHE_SIZE> zip_file_impl::s_cache;
std::mutex zip_file_impl::s_cache_mutex;



/*-------------------------------------------------
    zip_file_close - close a ZIP file and add it
    to the cache
-------------------------------------------------*/

void zip_file_impl::close(ptr &&zip) noexcept
{
	// if the filename isn't empty, the cached directory can be reused
	if (zip && !zip->m_filename.empty())
	{
		// close the open files
		osd_printf_verbose("unzip: closing archive file %s and sending to cache\n", zip->m_filename);
		zip->m_file.reset();

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
			osd_printf_verbose("unzip: removing %s from cache to make space\n", s_cache[cachenum]->m_filename);
			s_cache[cachenum].reset();
		}

		// move everyone else down and place us at the top
		for ( ; cachenum > 0; cachenum--)
			s_cache[cachenum] = std::move(s_cache[cachenum - 1]);
		s_cache[0] = std::move(zip);
	}

	// make sure it's cleaned up
	zip.reset();
}


/***************************************************************************
    CONTAINED FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_search - return the next matching
    entry in the ZIP
-------------------------------------------------*/

int zip_file_impl::search(std::uint32_t search_crc, std::string_view search_filename, bool matchcrc, bool matchname, bool partialpath) noexcept
{
	// if we're at or past the end, we're done
	while ((m_cd_pos + central_dir_entry_reader::minimum_length()) <= m_ecd.cd_size)
	{
		// make sure we have enough data
		central_dir_entry_reader const reader(&m_cd[0] + m_cd_pos);
		if (!reader.signature_correct() || ((m_cd_pos + reader.total_length()) > m_ecd.cd_size))
			break;

		// setting std::string can raise allocation exceptions
		try
		{
			// extract file header info
			file_header header;
			header.version_created     = reader.version_created();
			header.version_needed      = reader.version_needed();
			header.bit_flag            = reader.general_flag();
			header.compression         = reader.compression_method();
			header.crc                 = reader.crc32();
			header.compressed_length   = reader.compressed_size();
			header.uncompressed_length = reader.uncompressed_size();
			header.start_disk_number   = reader.start_disk();
			header.local_header_offset = reader.header_offset();

			// don't immediately decode DOS timestamp - it's expensive
			header.modified_date       = reader.modified_date();
			header.modified_time       = reader.modified_time();
			header.modified            = std::nullopt;

			// copy the filename
			bool is_utf8(general_flag_reader(header.bit_flag).utf8_encoding());
			reader.file_name(header.file_name);

			// walk the extra data
			for (auto extra = reader.extra_field(); extra.length_sufficient(); extra = extra.next())
			{
				// look for ZIP64 extended info
				if ((extra.header_id() == 0x0001) && (extra.data_size() >= zip64_ext_info_reader::minimum_length()))
				{
					zip64_ext_info_reader const ext64(reader, extra);
					if (extra.data_size() >= ext64.total_length())
					{
						header.compressed_length   = ext64.compressed_size();
						header.uncompressed_length = ext64.uncompressed_size();
						header.start_disk_number   = ext64.start_disk();
						header.local_header_offset = ext64.header_offset();
					}
				}

				// look for Info-ZIP UTF-8 path
				if (!is_utf8 && (extra.header_id() == 0x7075) && (extra.data_size() >= utf8_path_reader::minimum_length()))
				{
					utf8_path_reader const utf8path(extra);
					if (utf8path.version() == 1)
					{
						auto const addr(header.file_name.empty() ? nullptr : &header.file_name[0]);
						auto const length(header.file_name.empty() ? 0 : header.file_name.length() * sizeof(header.file_name[0]));
						auto const crc(crc32_creator::simple(addr, length));
						if (utf8path.name_crc32() == crc.m_raw)
						{
							utf8path.unicode_name(header.file_name);
							is_utf8 = true;
						}
					}
				}

				// look for NTFS extra field
				if ((extra.header_id() == 0x000a) && (extra.data_size() >= ntfs_reader::minimum_length()))
				{
					ntfs_reader const ntfs(extra);
					for (auto tag = ntfs.tag1(); tag.length_sufficient(); tag = tag.next())
					{
						if ((tag.tag() == 0x0001) && (tag.size() >= ntfs_times_reader::minimum_length()))
						{
							ntfs_times_reader const times(tag);
							ntfs_duration const ticks(times.mtime());
							try
							{
								header.modified = system_clock_time_point_from_ntfs_duration(ticks);
							}
							catch (...)
							{
								// out-of-range exception - let it fall back to DOS-style timestamp
							}
						}
					}
				}
			}

			// FIXME: if (!is_utf8) convert filename to UTF8 (assume CP437 or something)

			// chop off trailing slash for directory entries
			bool const is_dir(!header.file_name.empty() && (header.file_name.back() == '/'));
			if (is_dir)
				header.file_name.resize(header.file_name.length() - 1);

			// advance the position
			m_header = std::move(header);
			m_curr_is_dir = is_dir;
			m_cd_pos += reader.total_length();
		}
		catch (...)
		{
			break;
		}

		// quick return if not required to match on name
		if (!matchname)
		{
			if (!matchcrc || ((search_crc == m_header.crc) && !m_curr_is_dir))
				return 0;
		}
		else if (!m_curr_is_dir)
		{
			// check to see if it matches query
			auto const partialoffset = m_header.file_name.length() - search_filename.length();
			const bool namematch =
					(search_filename.length() == m_header.file_name.length()) &&
					(search_filename.empty() || !core_strnicmp(&search_filename[0], &m_header.file_name[0], search_filename.length()));
			bool const partialmatch =
					partialpath &&
					((m_header.file_name.length() > search_filename.length()) && (m_header.file_name[partialoffset - 1] == '/')) &&
					(search_filename.empty() || !core_strnicmp(&search_filename[0], &m_header.file_name[partialoffset], search_filename.length()));
			if ((!matchcrc || (search_crc == m_header.crc)) && (namematch || partialmatch))
				return 0;
		}
	}
	return -1;
}


/*-------------------------------------------------
    zip_file_decompress - decompress a file
    from a ZIP into the target buffer
-------------------------------------------------*/

std::error_condition zip_file_impl::decompress(void *buffer, std::size_t length) noexcept
{
	// if we don't have enough buffer, error
	if (length < m_header.uncompressed_length)
	{
		osd_printf_error("unzip: buffer too small to decompress %s from %s\n", m_header.file_name, m_filename);
		return archive_file::error::BUFFER_TOO_SMALL;
	}

	// make sure the info in the header aligns with what we know
	if (m_header.start_disk_number != m_ecd.disk_number)
	{
		osd_printf_error("unzip: %s does not reside in segment %s\n", m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}

	// get the compressed data offset
	std::uint64_t offset = 0;
	auto const ziperr = get_compressed_data_offset(offset);
	if (ziperr)
		return ziperr;

	// handle compression types
	switch (m_header.compression)
	{
	case 0:
		return decompress_data_type_0(offset, buffer, length);

	case 8:
		return decompress_data_type_8(offset, buffer, length);

	case 14:
		return decompress_data_type_14(offset, buffer, length);

	case 93:
		return decompress_data_type_93(offset, buffer, length);

	default:
		osd_printf_error(
				"unzip: %s in %s uses unsupported compression method %u\n",
				m_header.file_name, m_filename, m_header.compression);
		return archive_file::error::UNSUPPORTED;
	}
}



/***************************************************************************
    ZIP FILE PARSING
***************************************************************************/

/*-------------------------------------------------
    read_ecd - read the ECD data
-------------------------------------------------*/

std::error_condition zip_file_impl::read_ecd() noexcept
{
	// make sure the file handle is open
	auto const ziperr = reopen();
	if (ziperr)
		return ziperr;

	// we may need multiple tries
	std::error_condition filerr;
	std::size_t read_length;
	std::uint32_t buflen = 1024;
	while (buflen < 65536)
	{
		// max out the buffer length at the size of the file
		if (buflen > m_length)
			buflen = m_length;

		// allocate buffer
		std::unique_ptr<std::uint8_t []> buffer;
		buffer.reset(new (std::nothrow) std::uint8_t[buflen + 1]);
		if (!buffer)
		{
			osd_printf_error("unzip: %s failed to allocate memory for ECD search\n", m_filename);
			return std::errc::not_enough_memory;
		}

		// read in one buffers' worth of data
		std::tie(filerr, read_length) = read_at(*m_file, m_length - buflen, &buffer[0], buflen);
		if (filerr)
		{
			osd_printf_error(
					"unzip: error reading %s to search for ECD (%s:%d %s)\n",
					m_filename, filerr.category().name(), filerr.value(), filerr.message());
			return filerr;
		}
		if (read_length != buflen)
		{
			osd_printf_error("unzip: unexpectedly reached end-of-file while reading %s to search for ECD\n", m_filename);
			return archive_file::error::FILE_TRUNCATED;
		}

		// find the ECD signature
		std::int32_t offset;
		for (offset = buflen - ecd_reader::minimum_length(); offset >= 0; offset--)
		{
			ecd_reader reader(buffer.get() + offset);
			if (reader.signature_correct() && ((reader.total_length() + offset) <= buflen))
				break;
		}

		// if we found it, fill out the data
		if (offset >= 0)
		{
			osd_printf_verbose("unzip: found %s ECD at %d\n", m_filename, offset);

			// extract ECD info
			ecd_reader const ecd_rd(buffer.get() + offset);
			m_ecd.disk_number          = ecd_rd.this_disk_no();
			m_ecd.cd_start_disk_number = ecd_rd.dir_start_disk();
			m_ecd.cd_disk_entries      = ecd_rd.dir_disk_entries();
			m_ecd.cd_total_entries     = ecd_rd.dir_total_entries();
			m_ecd.cd_size              = ecd_rd.dir_size();
			m_ecd.cd_start_disk_offset = ecd_rd.dir_offset();

			// is the file too small to contain a ZIP64 ECD locator?
			if ((m_length - buflen + offset) < ecd64_locator_reader::minimum_length())
			{
				osd_printf_verbose("unzip: %s too small to contain ZIP64 ECD locator\n", m_filename);
				return std::error_condition();
			}

			// try to read the ZIP64 ECD locator
			std::tie(filerr, read_length) = read_at(
					*m_file,
					m_length - buflen + offset - ecd64_locator_reader::minimum_length(),
					&buffer[0],
					ecd64_locator_reader::minimum_length());
			if (filerr)
			{
				osd_printf_error(
						"unzip: error reading %s to search for ZIP64 ECD locator (%s:%d %s)\n",
						m_filename, filerr.category().name(), filerr.value(), filerr.message());
				return filerr;
			}
			if (read_length != ecd64_locator_reader::minimum_length())
			{
				osd_printf_error("unzip: unexpectedly reached end-of-file while reading %s to search for ZIP64 ECD locator\n", m_filename);
				return archive_file::error::FILE_TRUNCATED;
			}

			// if the signature isn't correct, it's not a ZIP64 archive
			ecd64_locator_reader const ecd64_loc_rd(buffer.get());
			if (!ecd64_loc_rd.signature_correct())
			{
				osd_printf_verbose("unzip: %s has no ZIP64 ECD locator\n", m_filename);
				return std::error_condition();
			}

			// check that the ZIP64 ECD is in this segment (assuming this segment is the last segment)
			if ((ecd64_loc_rd.ecd64_disk() + 1) != ecd64_loc_rd.total_disks())
			{
				osd_printf_error("unzip: %s ZIP64 ECD resides in a different segment\n", m_filename);
				return archive_file::error::UNSUPPORTED;
			}

			// try to read the ZIP64 ECD
			std::tie(filerr, read_length) = read_at(*m_file, ecd64_loc_rd.ecd64_offset(), &buffer[0], ecd64_reader::minimum_length());
			if (filerr)
			{
				osd_printf_error(
						"unzip: error reading %s ZIP64 ECD (%s:%d %s)\n",
						m_filename, filerr.category().name(), filerr.value(), filerr.message());
				return filerr;
			}
			if (read_length != ecd64_reader::minimum_length())
			{
				osd_printf_error("unzip: unexpectedly reached end-of-file while reading %s ZIP64 ECD\n", m_filename);
				return archive_file::error::FILE_TRUNCATED;
			}

			// check ZIP64 ECD
			ecd64_reader const ecd64_rd(buffer.get());
			if (!ecd64_rd.signature_correct())
			{
				osd_printf_error(
						"unzip: %s ZIP64 ECD has incorrect signature 0x%08x\n",
						m_filename, ecd64_rd.signature());
				return archive_file::error::BAD_SIGNATURE;
			}
			if (ecd64_rd.total_length() < ecd64_reader::minimum_length())
			{
				osd_printf_error("unzip: %s ZIP64 ECD appears to be too small\n", m_filename);
				return archive_file::error::UNSUPPORTED;
			}
			if (ecd64_rd.version_needed() > 63)
			{
				osd_printf_error(
						"unzip: %s ZIP64 ECD requires unsupported version %u.%u\n",
						m_filename, ecd64_rd.version_needed() / 10, ecd64_rd.version_needed() % 10);
				return archive_file::error::UNSUPPORTED;
			}
			osd_printf_verbose("unzip: found %s ZIP64 ECD\n", m_filename);

			// extract ZIP64 ECD info
			m_ecd.disk_number          = ecd64_rd.this_disk_no();
			m_ecd.cd_start_disk_number = ecd64_rd.dir_start_disk();
			m_ecd.cd_disk_entries      = ecd64_rd.dir_disk_entries();
			m_ecd.cd_total_entries     = ecd64_rd.dir_total_entries();
			m_ecd.cd_size              = ecd64_rd.dir_size();
			m_ecd.cd_start_disk_offset = ecd64_rd.dir_offset();

			return std::error_condition();
		}

		// didn't find it; free this buffer and expand our search
		if (buflen < m_length)
		{
			buflen *= 2;
		}
		else
		{
			osd_printf_error("unzip: %s couldn't find ECD\n", m_filename);
			return archive_file::error::BAD_SIGNATURE;
		}
	}
	osd_printf_error("unzip: %s couldn't find ECD in last 64KiB\n", m_filename);
	return archive_file::error::UNSUPPORTED; // TODO: revisit this error code
}


/*-------------------------------------------------
    get_compressed_data_offset - return the
    offset of the compressed data
-------------------------------------------------*/

std::error_condition zip_file_impl::get_compressed_data_offset(std::uint64_t &offset) noexcept
{
	// don't support a number of features
	general_flag_reader const flags(m_header.bit_flag);
	if (m_header.start_disk_number != m_ecd.disk_number)
	{
		osd_printf_error("unzip: %s does not reside in segment %s\n", m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}
	if (m_header.version_needed > 63)
	{
		osd_printf_error(
				"unzip: %s in %s requires unsupported version %u.%u\n",
				m_header.file_name, m_filename, m_header.version_needed / 10, m_header.version_needed % 10);
		return archive_file::error::UNSUPPORTED;
	}
	if (flags.encrypted() || flags.strong_encryption())
	{
		osd_printf_error("unzip: %s in %s is encrypted\n", m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}
	if (flags.patch_data())
	{
		osd_printf_error("unzip: %s in %s is compressed patch data\n", m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}

	// make sure the file handle is open
	auto const ziperr = reopen();
	if (ziperr)
		return ziperr;

	// now go read the fixed-sized part of the local file header
	auto const [filerr, read_length] = read_at(*m_file, m_header.local_header_offset, &m_buffer[0], local_file_header_reader::minimum_length());
	if (filerr)
	{
		osd_printf_error(
				"unzip: error reading local file header for %s in %s (%s:%d %s)\n",
				m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
		return filerr;
	}
	if (read_length != local_file_header_reader::minimum_length())
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading local file header for %s in %s\n",
				m_header.file_name, m_filename);
		return archive_file::error::FILE_TRUNCATED;
	}

	// compute the final offset
	local_file_header_reader reader(&m_buffer[0]);
	if (!reader.signature_correct())
	{
		osd_printf_error(
				"unzip: local file header for %s in %s has incorrect signature %04x\n",
				m_header.file_name, m_filename, reader.signature());
		return archive_file::error::BAD_SIGNATURE;
	}
	offset = m_header.local_header_offset + reader.total_length();

	return std::error_condition();
}



/***************************************************************************
    DECOMPRESSION INTERFACES
***************************************************************************/

/*-------------------------------------------------
    decompress_data_type_0 - "decompress"
    type 0 data (which is uncompressed)
-------------------------------------------------*/

std::error_condition zip_file_impl::decompress_data_type_0(std::uint64_t offset, void *buffer, std::size_t length) noexcept
{
	// the data is uncompressed; just read it
	auto const [filerr, read_length] = read_at(*m_file, offset, buffer, m_header.compressed_length);
	if (filerr)
	{
		osd_printf_error(
				"unzip: error reading %s from %s (%s:%d %s)\n",
				m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
		return filerr;
	}
	else if (read_length != m_header.compressed_length)
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading %s from %s\n",
				m_header.file_name, m_filename);
		return archive_file::error::FILE_TRUNCATED;
	}
	else
	{
		return std::error_condition();
	}
}


/*-------------------------------------------------
    decompress_data_type_8 - decompress
    type 8 data (which is deflated)
-------------------------------------------------*/

std::error_condition zip_file_impl::decompress_data_type_8(std::uint64_t offset, void *buffer, std::size_t length) noexcept
{
	auto const convert_zerr =
			[] (int e) -> std::error_condition
			{
				switch (e)
				{
				case Z_OK:
					return std::error_condition();
				case Z_ERRNO:
					return std::error_condition(errno, std::generic_category());
				case Z_MEM_ERROR:
					return std::errc::not_enough_memory;
				default:
					return archive_file::error::DECOMPRESS_ERROR;
				}
			};
	std::uint64_t input_remaining(m_header.compressed_length);
	int zerr;

	// reset the stream
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_out = reinterpret_cast<Bytef *>(buffer);
	stream.avail_out = length;

	// initialize the decompressor
	zerr = inflateInit2(&stream, -MAX_WBITS);
	if (zerr != Z_OK)
	{
		auto result = convert_zerr(zerr);
		osd_printf_error(
				"unzip: error allocating zlib stream to inflate %s from %s (%d)\n",
				m_header.file_name, m_filename, zerr);
		return result;
	}

	// loop until we're done
	while (true)
	{
		// read in the next chunk of data
		auto const [filerr, read_length] = read_at(
				*m_file,
				offset,
				&m_buffer[0],
				std::size_t(std::min<std::uint64_t>(input_remaining, m_buffer.size())));
		if (filerr)
		{
			osd_printf_error(
					"unzip: error reading compressed data for %s in %s (%s:%d %s)\n",
					m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
			inflateEnd(&stream);
			return filerr;
		}
		offset += read_length;

		// if we read nothing, but still have data left, the file is truncated
		if (!read_length && input_remaining)
		{
			osd_printf_error(
					"unzip: unexpectedly reached end-of-file while reading compressed data for %s in %s\n",
					m_header.file_name, m_filename);
			inflateEnd(&stream);
			return archive_file::error::FILE_TRUNCATED;
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
		{
			break;
		}
		else if (zerr != Z_OK)
		{
			auto result = convert_zerr(zerr);
			osd_printf_error(
					"unzip: error inflating %s from %s (%d)\n",
					m_header.file_name, m_filename, zerr);
			inflateEnd(&stream);
			return result;
		}
	}

	// finish decompression
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
	{
		auto result = convert_zerr(zerr);
		osd_printf_error(
				"unzip: error finishing inflation of %s from %s (%d)\n",
				m_header.file_name, m_filename, zerr);
		return result;
	}

	// if anything looks funny, report an error
	if (stream.avail_out || input_remaining)
	{
		osd_printf_error(
				"unzip: inflation of %s from %s doesn't appear to have completed correctly\n",
				m_header.file_name, m_filename);
		return archive_file::error::DECOMPRESS_ERROR;
	}

	return std::error_condition();
}


/*-------------------------------------------------
    decompress_data_type_14 - decompress
    type 14 data (LZMA)
-------------------------------------------------*/

std::error_condition zip_file_impl::decompress_data_type_14(std::uint64_t offset, void *buffer, std::size_t length) noexcept
{
	// two-byte version
	// two-byte properties size (little-endian)
	// properties
	// compressed data

	assert(4 <= m_buffer.size());
	assert(LZMA_PROPS_SIZE <= m_buffer.size());

	bool const eos_mark(general_flag_reader(m_header.bit_flag).lzma_eos_mark());
	Byte *const output(reinterpret_cast<Byte *>(buffer));
	SizeT output_remaining(length);
	std::uint64_t input_remaining(m_header.compressed_length);

	std::size_t read_length;
	std::error_condition filerr;
	SRes lzerr;
	ELzmaStatus lzstatus(LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK);

	// reset the stream
	ISzAlloc alloc_imp;
	alloc_imp.Alloc = [] (ISzAllocPtr p, std::size_t size) -> void * { return size ? std::malloc(size) : nullptr; };
	alloc_imp.Free = [] (ISzAllocPtr p, void *address) -> void { std::free(address); };
	CLzmaDec stream;
	LzmaDec_Construct(&stream);

	// need to read LZMA properties before we can initialise the decompressor
	if (4 > input_remaining)
	{
		osd_printf_error(
				"unzip:compressed data for %s in %s is too small to hold LZMA properties header\n",
				m_header.file_name, m_filename);
		return archive_file::error::DECOMPRESS_ERROR;
	}
	std::tie(filerr, read_length) = read_at(*m_file, offset, &m_buffer[0], 4);
	if (filerr)
	{
		osd_printf_error(
				"unzip: error reading LZMA properties header for %s in %s (%s:%d %s)\n",
				m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
		return filerr;
	}
	offset += read_length;
	input_remaining -= read_length;
	if (4 != read_length)
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading LZMA properties header for %s in %s\n",
				m_header.file_name, m_filename);
		return archive_file::error::FILE_TRUNCATED;
	}
	std::uint16_t const props_size(get_u16le(&m_buffer[2]));
	if (props_size > m_buffer.size())
	{
		osd_printf_error(
				"unzip: %s in %s has excessively large LZMA properties\n",
				m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}
	else if (props_size > input_remaining)
	{
		osd_printf_error(
				"unzip:compressed data for %s in %s is too small to hold LZMA properties\n",
				m_header.file_name, m_filename);
		return archive_file::error::DECOMPRESS_ERROR;
	}
	std::tie(filerr, read_length) = read_at(*m_file, offset, &m_buffer[0], props_size);
	if (filerr)
	{
		osd_printf_error(
				"unzip: error reading LZMA properties for %s in %s (%s:%d %s)\n",
				m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
		return filerr;
	}
	offset += read_length;
	input_remaining -= read_length;
	if (props_size != read_length)
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading LZMA properties for %s in %s\n",
				m_header.file_name, m_filename);
		return archive_file::error::FILE_TRUNCATED;
	}

	// initialize the decompressor
	lzerr = LzmaDec_Allocate(&stream, &m_buffer[0], props_size, &alloc_imp);
	if (SZ_ERROR_MEM == lzerr)
	{
		osd_printf_error(
				"unzip: memory error allocating LZMA decoder to decompress %s from %s\n",
				m_header.file_name, m_filename);
		return std::errc::not_enough_memory;
	}
	else if (SZ_ERROR_UNSUPPORTED)
	{
		osd_printf_error(
				"unzip: LZMA decoder does not support properties for %s in %s\n",
				m_header.file_name, m_filename);
		return archive_file::error::UNSUPPORTED;
	}
	else if (SZ_OK != lzerr)
	{
		osd_printf_error(
				"unzip: error allocating LZMA decoder to decompress %s from %s (%d)\n",
				m_header.file_name, m_filename, int(lzerr));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	LzmaDec_Init(&stream);

	// loop until we're done
	while (0 < input_remaining)
	{
		// read in the next chunk of data
		std::tie(filerr, read_length) = read_at(
				*m_file,
				offset,
				&m_buffer[0],
				std::size_t((std::min<std::uint64_t>)(input_remaining, m_buffer.size())));
		if (filerr)
		{
			osd_printf_error(
					"unzip: error reading compressed data for %s in %s (%s)\n",
					m_header.file_name, m_filename);
			LzmaDec_Free(&stream, &alloc_imp);
			return filerr;
		}
		offset += read_length;
		input_remaining -= read_length;

		// if we read nothing, but still have data left, the file is truncated
		if (!read_length && input_remaining)
		{
			osd_printf_error(
					"unzip: unexpectedly reached end-of-file while reading compressed data for %s in %s\n",
					m_header.file_name, m_filename);
			LzmaDec_Free(&stream, &alloc_imp);
			return archive_file::error::FILE_TRUNCATED;
		}

		// now decompress
		SizeT len(read_length);
		// FIXME: is the input length really an in/out parameter or not?
		// At least on reaching the end of the input, the input length doesn't seem to get updated
		lzerr = LzmaDec_DecodeToBuf(
				&stream,
				output + length - output_remaining,
				&output_remaining,
				reinterpret_cast<Byte const *>(&m_buffer[0]),
				&len,
				(!input_remaining && eos_mark) ? LZMA_FINISH_END :  LZMA_FINISH_ANY,
				&lzstatus);
		if (SZ_OK != lzerr)
		{
			osd_printf_error("unzip: error decoding LZMA data for %s in %s (%d)\n", m_header.file_name, m_filename, int(lzerr));
			LzmaDec_Free(&stream, &alloc_imp);
			return archive_file::error::DECOMPRESS_ERROR;
		}
	}

	// finish decompression
	LzmaDec_Free(&stream, &alloc_imp);

	// if anything looks funny, report an error
	if (LZMA_STATUS_FINISHED_WITH_MARK == lzstatus)
	{
		return std::error_condition();
	}
	else if (eos_mark)
	{
		osd_printf_error(
				"unzip: LZMA end mark not found for %s in %s (%d)\n",
				m_header.file_name, m_filename, int(lzstatus));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	else if (LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK != lzstatus)
	{
		osd_printf_error(
				"unzip: LZMA decompression of %s from %s doesn't appear to have completed correctly (%d)\n",
				m_header.file_name, m_filename, int(lzstatus));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	else
	{
		return std::error_condition();
	}
}


/*-------------------------------------------------
    decompress_data_type_93 - decompress
    type 14 data (Zstandard)
-------------------------------------------------*/

std::error_condition zip_file_impl::decompress_data_type_93(std::uint64_t offset, void *buffer, std::size_t length) noexcept
{
	// create decompression stream
	ZSTD_DStream *const stream(ZSTD_createDStream());
	if (!stream)
	{
		osd_printf_error(
				"unzip: error allocating Zstandard stream to decompress %s from %s\n",
				m_header.file_name, m_filename);
		return std::errc::not_enough_memory;
	}

	// loop until we're done
	std::uint64_t input_remaining(m_header.compressed_length);
	while (input_remaining && length)
	{
		// read in the next chunk of data
		auto const [filerr, read_length] = read_at(
				*m_file,
				offset,
				&m_buffer[0],
				std::size_t(std::min<std::uint64_t>(input_remaining, m_buffer.size())));
		if (filerr)
		{
			osd_printf_error(
					"unzip: error reading compressed data for %s in %s (%s:%d %s)\n",
					m_header.file_name, m_filename, filerr.category().name(), filerr.value(), filerr.message());
			ZSTD_freeDStream(stream);
			return filerr;
		}
		offset += read_length;

		// if we read nothing, but still have data left, the file is truncated
		if (!read_length && input_remaining)
		{
			osd_printf_error(
					"unzip: unexpectedly reached end-of-file while reading compressed data for %s in %s\n",
					m_header.file_name, m_filename);
			ZSTD_freeDStream(stream);
			return archive_file::error::FILE_TRUNCATED;
		}

		// fill out the input data
		ZSTD_inBuffer input{ &m_buffer[0], read_length, 0 };
		input_remaining -= read_length;

		// now decompress
		while ((input.pos < input.size) && length)
		{
			ZSTD_outBuffer output{ buffer, length, 0 };
			auto const result(ZSTD_decompressStream(stream, &output, &input));
			if (ZSTD_isError(result))
			{
				osd_printf_error(
						"unzip: error decompressing %s from %s (%u: %s)\n",
						m_header.file_name, m_filename, result, ZSTD_getErrorName(result));
				ZSTD_freeDStream(stream);
				return archive_file::error::DECOMPRESS_ERROR;
			}
			buffer = reinterpret_cast<std::uint8_t *>(buffer) + output.pos;
			length -= output.pos;
		}
	}

	// free stream
	ZSTD_freeDStream(stream);

	// if anything looks funny, report an error
	if (length || input_remaining)
	{
		osd_printf_error(
				"unzip: decompression of %s from %s doesn't appear to have completed correctly\n",
				m_header.file_name, m_filename);
		return archive_file::error::DECOMPRESS_ERROR;
	}

	return std::error_condition();
}

} // anonymous namespace



/***************************************************************************
    un7z.cpp TRAMPOLINES
***************************************************************************/

void m7z_file_cache_clear() noexcept;



/***************************************************************************
    ZIP FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_open - opens a ZIP file for reading
-------------------------------------------------*/

std::error_condition archive_file::open_zip(std::string_view filename, ptr &result) noexcept
{
	// ensure we start with a nullptr result
	result.reset();

	// see if we are in the cache, and reopen if so
	zip_file_impl::ptr newimpl(zip_file_impl::find_cached(filename));

	if (!newimpl)
	{
		// allocate memory for the zip_file structure
		try { newimpl = std::make_unique<zip_file_impl>(std::string(filename)); }
		catch (...) { return std::errc::not_enough_memory; }
		auto const err = newimpl->initialize();
		if (err)
			return err;
	}

	// allocate the archive API wrapper
	result.reset(new (std::nothrow) zip_file_wrapper(std::move(newimpl)));
	if (result)
	{
		return std::error_condition();
	}
	else
	{
		zip_file_impl::close(std::move(newimpl));
		return std::errc::not_enough_memory;
	}
}

std::error_condition archive_file::open_zip(random_read::ptr &&file, ptr &result) noexcept
{
	// ensure we start with a nullptr result
	result.reset();

	// allocate memory for the zip_file structure
	zip_file_impl::ptr newimpl(new (std::nothrow) zip_file_impl(std::move(file)));
	if (!newimpl)
		return std::errc::not_enough_memory;
	auto const err = newimpl->initialize();
	if (err)
		return err;

	// allocate the archive API wrapper
	result.reset(new (std::nothrow) zip_file_wrapper(std::move(newimpl)));
	if (result)
	{
		return std::error_condition();
	}
	else
	{
		zip_file_impl::close(std::move(newimpl));
		return std::errc::not_enough_memory;
	}
}


/*-------------------------------------------------
    zip_file_cache_clear - clear the ZIP file
    cache and free all memory
-------------------------------------------------*/

void archive_file::cache_clear() noexcept
{
	zip_file_impl::cache_clear();
	m7z_file_cache_clear();
}


archive_file::~archive_file()
{
}


/*-------------------------------------------------
    archive_category - gets the archive error
    category instance
-------------------------------------------------*/

std::error_category const &archive_category() noexcept
{
	return f_archive_category_instance;
}

} // namespace util
