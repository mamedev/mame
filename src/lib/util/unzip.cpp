// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    unzip.c

    Functions to manipulate data within ZIP files.

***************************************************************************/

#include "unzip.h"

#include "corestr.h"
#include "hashing.h"
#include "osdcore.h"
#include "timeconv.h"

#include "lzma/C/LzmaDec.h"

#include <zlib.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstring>
#include <cstdlib>
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
		, m_curr_is_dir(false)
		, m_buffer()
	{
		std::fill(m_buffer.begin(), m_buffer.end(), 0);
	}

	static ptr find_cached(const std::string &filename)
	{
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		for (std::size_t cachenum = 0; cachenum < s_cache.size(); cachenum++)
		{
			// if we have a valid entry and it matches our filename, use it and remove from the cache
			if (s_cache[cachenum] && (filename == s_cache[cachenum]->m_filename))
			{
				ptr result;
				std::swap(s_cache[cachenum], result);
				osd_printf_verbose("unzip: found %s in cache\n", filename);
				return result;
			}
		}
		return ptr();
	}
	static void close(ptr &&zip);
	static void cache_clear()
	{
		// clear call cache entries
		std::lock_guard<std::mutex> guard(s_cache_mutex);
		for (std::size_t cachenum = 0; cachenum < s_cache.size(); s_cache[cachenum++].reset()) { }
	}

	archive_file::error initialize()
	{
		// read ecd data
		auto const ziperr = read_ecd();
		if (ziperr != archive_file::error::NONE)
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
			return archive_file::error::OUT_OF_MEMORY;
		}

		// read the central directory
		auto cd_remaining(m_ecd.cd_size);
		std::size_t cd_offs(0);
		while (cd_remaining)
		{
			std::uint32_t const chunk(std::uint32_t((std::min<std::uint64_t>)(std::numeric_limits<std::uint32_t>::max(), cd_remaining)));
			std::uint32_t read_length(0);
			auto const filerr = m_file->read(&m_cd[cd_offs], m_ecd.cd_start_disk_offset + cd_offs, chunk, read_length);
			if (filerr != osd_file::error::NONE)
			{
				osd_printf_error("unzip: %s error reading central directory (%d)\n", m_filename, int(filerr));
				return archive_file::error::FILE_ERROR;
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

		return archive_file::error::NONE;
	}

	int first_file()
	{
		m_cd_pos = 0;
		return search(0, std::string(), false, false, false);
	}
	int next_file()
	{
		return search(0, std::string(), false, false, false);
	}

	int search(std::uint32_t crc)
	{
		m_cd_pos = 0;
		return search(crc, std::string(), true, false, false);
	}
	int search(const std::string &filename, bool partialpath)
	{
		m_cd_pos = 0;
		return search(0, filename, false, true, partialpath);
	}
	int search(std::uint32_t crc, const std::string &filename, bool partialpath)
	{
		m_cd_pos = 0;
		return search(crc, filename, true, true, partialpath);
	}

	bool current_is_directory() const { return m_curr_is_dir; }
	const std::string &current_name() const { return m_header.file_name; }
	std::uint64_t current_uncompressed_length() const { return m_header.uncompressed_length; }
	std::chrono::system_clock::time_point current_last_modified() const
	{
		if (!m_header.modified_cached)
		{
			m_header.modified = decode_dos_time(m_header.modified_date, m_header.modified_time);
			m_header.modified_cached = true;
		}
		return m_header.modified;
	}
	std::uint32_t current_crc() const { return m_header.crc; }

	archive_file::error decompress(void *buffer, std::uint32_t length);

private:
	zip_file_impl(const zip_file_impl &) = delete;
	zip_file_impl(zip_file_impl &&) = delete;
	zip_file_impl &operator=(const zip_file_impl &) = delete;
	zip_file_impl &operator=(zip_file_impl &&) = delete;

	int search(std::uint32_t search_crc, const std::string &search_filename, bool matchcrc, bool matchname, bool partialpath);

	archive_file::error reopen()
	{
		if (!m_file)
		{
			auto const filerr = osd_file::open(m_filename, OPEN_FLAG_READ, m_file, m_length);
			if (filerr != osd_file::error::NONE)
			{
				// this would spam every time it looks for a non-existent archive, which is a lot
				//osd_printf_error("unzip: error reopening archive file %s (%d)\n", m_filename, int(filerr));
				return archive_file::error::FILE_ERROR;
			}
			else
			{
				osd_printf_verbose("unzip: opened archive file %s\n", m_filename);
			}
		}
		return archive_file::error::NONE;
	}

	static std::chrono::system_clock::time_point decode_dos_time(std::uint16_t date, std::uint16_t time)
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
	archive_file::error read_ecd();
	archive_file::error get_compressed_data_offset(std::uint64_t &offset);

	// decompression interfaces
	archive_file::error decompress_data_type_0(std::uint64_t offset, void *buffer, std::uint32_t length);
	archive_file::error decompress_data_type_8(std::uint64_t offset, void *buffer, std::uint32_t length);
	archive_file::error decompress_data_type_14(std::uint64_t offset, void *buffer, std::uint32_t length);

	struct file_header
	{
		std::uint16_t                                   version_created;        // version made by
		std::uint16_t                                   version_needed;         // version needed to extract
		std::uint16_t                                   bit_flag;               // general purpose bit flag
		std::uint16_t                                   compression;            // compression method
		mutable std::chrono::system_clock::time_point   modified;               // last mod file date/time
		std::uint32_t                                   crc;                    // crc-32
		std::uint64_t                                   compressed_length;      // compressed size
		std::uint64_t                                   uncompressed_length;    // uncompressed size
		std::uint32_t                                   start_disk_number;      // disk number start
		std::uint64_t                                   local_header_offset;    // relative offset of local header
		std::string                                     file_name;              // file name

		std::uint16_t                                   modified_date, modified_time;
		mutable bool                                    modified_cached;
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
	osd_file::ptr               m_file;                     // OSD file handle
	std::uint64_t               m_length;                   // length of zip file

	ecd                         m_ecd;                      // end of central directory

	std::vector<std::uint8_t>   m_cd;                       // central directory raw data
	std::uint32_t               m_cd_pos;                   // position in central directory
	file_header                 m_header;                   // current file header
	bool                        m_curr_is_dir;              // current file is directory

	std::array<std::uint8_t, DECOMPRESS_BUFSIZE> m_buffer;  // buffer for decompression
};


class zip_file_wrapper : public archive_file
{
public:
	zip_file_wrapper(zip_file_impl::ptr &&impl) : m_impl(std::move(impl)) { assert(m_impl); }
	virtual ~zip_file_wrapper() { zip_file_impl::close(std::move(m_impl)); }

	virtual int first_file() override { return m_impl->first_file(); }
	virtual int next_file() override { return m_impl->next_file(); }

	virtual int search(std::uint32_t crc) override
	{
		return m_impl->search(crc);
	}
	virtual int search(const std::string &filename, bool partialpath) override
	{
		return m_impl->search(filename, partialpath);
	}
	virtual int search(std::uint32_t crc, const std::string &filename, bool partialpath) override
	{
		return m_impl->search(crc, filename, partialpath);
	}

	virtual bool current_is_directory() const override { return m_impl->current_is_directory(); }
	virtual const std::string &current_name() const override { return m_impl->current_name(); }
	virtual std::uint64_t current_uncompressed_length() const override { return m_impl->current_uncompressed_length(); }
	virtual std::chrono::system_clock::time_point current_last_modified() const override { return m_impl->current_last_modified(); }
	virtual std::uint32_t current_crc() const override { return m_impl->current_crc(); }

	virtual error decompress(void *buffer, std::uint32_t length) override { return m_impl->decompress(buffer, length); }

private:
	zip_file_impl::ptr m_impl;
};


class reader_base
{
protected:
	reader_base(void const *buf) : m_buffer(reinterpret_cast<std::uint8_t const *>(buf)) { }

	std::uint8_t read_byte(std::size_t offs) const
	{
		return m_buffer[offs];
	}
	std::uint16_t read_word(std::size_t offs) const
	{
		return
				(std::uint16_t(m_buffer[offs + 1]) << 8) |
				(std::uint16_t(m_buffer[offs + 0]) << 0);
	}
	std::uint32_t read_dword(std::size_t offs) const
	{
		return
				(std::uint32_t(m_buffer[offs + 3]) << 24) |
				(std::uint32_t(m_buffer[offs + 2]) << 16) |
				(std::uint32_t(m_buffer[offs + 1]) << 8) |
				(std::uint32_t(m_buffer[offs + 0]) << 0);
	}
	std::uint64_t read_qword(std::size_t offs) const
	{
		return
				(std::uint64_t(m_buffer[offs + 7]) << 56) |
				(std::uint64_t(m_buffer[offs + 6]) << 48) |
				(std::uint64_t(m_buffer[offs + 5]) << 40) |
				(std::uint64_t(m_buffer[offs + 4]) << 32) |
				(std::uint64_t(m_buffer[offs + 3]) << 24) |
				(std::uint64_t(m_buffer[offs + 2]) << 16) |
				(std::uint64_t(m_buffer[offs + 1]) << 8) |
				(std::uint64_t(m_buffer[offs + 0]) << 0);
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
	extra_field_reader(void const *buf, std::size_t len) : reader_base(buf), m_length(len) { }

	std::uint16_t         header_id() const { return read_word(0x00); }
	std::uint16_t         data_size() const { return read_word(0x02); }
	void const *          data() const      { return m_buffer + 0x04; }
	extra_field_reader    next() const      { return extra_field_reader(m_buffer + total_length(), m_length - total_length()); }

	bool length_sufficient() const { return (m_length >= minimum_length()) && (m_length >= total_length()); }

	std::size_t total_length() const { return minimum_length() + data_size(); }
	static std::size_t minimum_length() { return 0x04; }

private:
	std::size_t m_length;
};


class local_file_header_reader : private reader_base
{
public:
	local_file_header_reader(void const *buf) : reader_base(buf) { }

	std::uint32_t       signature() const                       { return read_dword(0x00); }
	std::uint8_t        version_needed() const                  { return m_buffer[0x04]; }
	std::uint8_t        os_needed() const                       { return m_buffer[0x05]; }
	std::uint16_t       general_flag() const                    { return read_word(0x06); }
	std::uint16_t       compression_method() const              { return read_word(0x08); }
	std::uint16_t       modified_time() const                   { return read_word(0x0a); }
	std::uint16_t       modified_date() const                   { return read_word(0x0c); }
	std::uint32_t       crc32() const                           { return read_dword(0x0e); }
	std::uint32_t       compressed_size() const                 { return read_dword(0x12); }
	std::uint32_t       uncompressed_size() const               { return read_dword(0x16); }
	std::uint16_t       file_name_length() const                { return read_word(0x1a); }
	std::uint16_t       extra_field_length() const              { return read_word(0x1c); }
	std::string         file_name() const                       { return read_string(0x1e, file_name_length()); }
	void                file_name(std::string &result) const    { read_string(result, 0x1e, file_name_length()); }
	extra_field_reader  extra_field() const                     { return extra_field_reader(m_buffer + 0x1e + file_name_length(), extra_field_length()); }

	bool                signature_correct() const               { return signature() == 0x04034b50; }

	std::size_t total_length() const { return minimum_length() + file_name_length() + extra_field_length(); }
	static std::size_t minimum_length() { return 0x1e; }
};


class central_dir_entry_reader : private reader_base
{
public:
	central_dir_entry_reader(void const *buf) : reader_base(buf) { }

	std::uint32_t       signature() const                       { return read_dword(0x00); }
	std::uint8_t        version_created() const                 { return m_buffer[0x04]; }
	std::uint8_t        os_created() const                      { return m_buffer[0x05]; }
	std::uint8_t        version_needed() const                  { return m_buffer[0x06]; }
	std::uint8_t        os_needed() const                       { return m_buffer[0x07]; }
	std::uint16_t       general_flag() const                    { return read_word(0x08); }
	std::uint16_t       compression_method() const              { return read_word(0x0a); }
	std::uint16_t       modified_time() const                   { return read_word(0x0c); }
	std::uint16_t       modified_date() const                   { return read_word(0x0e); }
	std::uint32_t       crc32() const                           { return read_dword(0x10); }
	std::uint32_t       compressed_size() const                 { return read_dword(0x14); }
	std::uint32_t       uncompressed_size() const               { return read_dword(0x18); }
	std::uint16_t       file_name_length() const                { return read_word(0x1c); }
	std::uint16_t       extra_field_length() const              { return read_word(0x1e); }
	std::uint16_t       file_comment_length() const             { return read_word(0x20); }
	std::uint16_t       start_disk() const                      { return read_word(0x22); }
	std::uint16_t       int_file_attr() const                   { return read_word(0x24); }
	std::uint32_t       ext_file_attr() const                   { return read_dword(0x26); }
	std::uint32_t       header_offset() const                   { return read_dword(0x2a); }
	std::string         file_name() const                       { return read_string(0x2e, file_name_length()); }
	void                file_name(std::string &result) const    { read_string(result, 0x2e, file_name_length()); }
	extra_field_reader  extra_field() const                     { return extra_field_reader(m_buffer + 0x2e + file_name_length(), extra_field_length()); }
	std::string         file_comment() const                    { return read_string(0x2e + file_name_length() + extra_field_length(), file_comment_length()); }
	void                file_comment(std::string &result) const { read_string(result, 0x2e + file_name_length() + extra_field_length(), file_comment_length()); }

	bool                signature_correct() const               { return signature() == 0x02014b50; }

	std::size_t total_length() const { return minimum_length() + file_name_length() + extra_field_length() + file_comment_length(); }
	static std::size_t minimum_length() { return 0x2e; }
};


class ecd64_reader : private reader_base
{
public:
	ecd64_reader(void const *buf) : reader_base(buf) { }

	std::uint32_t   signature() const           { return read_dword(0x00); }
	std::uint64_t   ecd64_size() const          { return read_qword(0x04); }
	std::uint8_t    version_created() const     { return m_buffer[0x0c]; }
	std::uint8_t    os_created() const          { return m_buffer[0x0d]; }
	std::uint8_t    version_needed() const      { return m_buffer[0x0e]; }
	std::uint8_t    os_needed() const           { return m_buffer[0x0f]; }
	std::uint32_t   this_disk_no() const        { return read_dword(0x10); }
	std::uint32_t   dir_start_disk() const      { return read_dword(0x14); }
	std::uint64_t   dir_disk_entries() const    { return read_qword(0x18); }
	std::uint64_t   dir_total_entries() const   { return read_qword(0x20); }
	std::uint64_t   dir_size() const            { return read_qword(0x28); }
	std::uint64_t   dir_offset() const          { return read_qword(0x30); }
	void const *    extensible_data() const     { return m_buffer + 0x38; }

	bool            signature_correct() const   { return signature() == 0x06064b50; }

	std::size_t total_length() const { return 0x0c + ecd64_size(); }
	static std::size_t minimum_length() { return 0x38; }
};


class ecd64_locator_reader : private reader_base
{
public:
	ecd64_locator_reader(void const *buf) : reader_base(buf) { }

	std::uint32_t   signature() const           { return read_dword(0x00); }
	std::uint32_t   ecd64_disk() const          { return read_dword(0x04); }
	std::uint64_t   ecd64_offset() const        { return read_qword(0x08); }
	std::uint32_t   total_disks() const         { return read_dword(0x10); }

	bool            signature_correct() const   { return signature() == 0x07064b50; }

	std::size_t total_length() const { return minimum_length(); }
	static std::size_t minimum_length() { return 0x14; }
};


class ecd_reader : private reader_base
{
public:
	ecd_reader(void const *buf) : reader_base(buf) { }

	std::uint32_t   signature() const                   { return read_dword(0x00); }
	std::uint16_t   this_disk_no() const                { return read_word(0x04); }
	std::uint16_t   dir_start_disk() const              { return read_word(0x06); }
	std::uint16_t   dir_disk_entries() const            { return read_word(0x08); }
	std::uint16_t   dir_total_entries() const           { return read_word(0x0a); }
	std::uint32_t   dir_size() const                    { return read_dword(0x0c); }
	std::uint32_t   dir_offset() const                  { return read_dword(0x10); }
	std::uint16_t   comment_length() const              { return read_word(0x14); }
	std::string     comment() const                     { return read_string(0x16, comment_length()); }
	void            comment(std::string &result) const  { read_string(result, 0x16, comment_length()); }

	bool            signature_correct() const           { return signature() == 0x06054b50; }

	std::size_t total_length() const { return minimum_length() + comment_length(); }
	static std::size_t minimum_length() { return 0x16; }
};


class zip64_ext_info_reader : private reader_base
{
public:
	template <typename T>
	zip64_ext_info_reader(
			T const &header,
			extra_field_reader const &field)
		: reader_base(field.data())
		, m_uncompressed_size(header.uncompressed_size())
		, m_compressed_size(header.compressed_size())
		, m_header_offset(header.header_offset())
		, m_start_disk(header.start_disk())
		, m_offs_compressed_size(~m_uncompressed_size ? 0 : 8)
		, m_offs_header_offset(m_offs_compressed_size + (~m_compressed_size ? 0 : 8))
		, m_offs_start_disk(m_offs_header_offset + (~m_header_offset ? 0 : 8))
		, m_offs_end(m_offs_start_disk + (~m_start_disk ? 0 : 4))
	{
	}

	std::uint64_t   uncompressed_size() const   { return ~m_uncompressed_size ? m_uncompressed_size : read_qword(0x00); }
	std::uint64_t   compressed_size() const     { return ~m_compressed_size ? m_compressed_size : read_qword(m_offs_compressed_size); }
	std::uint64_t   header_offset() const       { return ~m_header_offset ? m_header_offset : read_qword(m_offs_header_offset); }
	std::uint32_t   start_disk() const          { return ~m_start_disk ? m_start_disk : read_dword(m_offs_start_disk); }

	std::size_t total_length() const { return minimum_length() + m_offs_end; }
	static std::size_t minimum_length() { return 0x00; }

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
	utf8_path_reader(extra_field_reader const &field) : reader_base(field.data()), m_length(field.data_size()) { }

	std::uint8_t    version() const                         { return m_buffer[0]; }
	std::uint32_t   name_crc32() const                      { return read_dword(0x01); }
	std::string     unicode_name() const                    { return read_string(0x05, m_length - 0x05); }
	void            unicode_name(std::string &result) const { return read_string(result, 0x05, m_length - 0x05); }

	std::size_t total_length() const { return m_length; }
	static std::size_t minimum_length() { return 0x05; }

private:
	std::size_t m_length;
};


class ntfs_tag_reader : private reader_base
{
public:
	ntfs_tag_reader(void const *buf, std::size_t len) : reader_base(buf), m_length(len) { }

	std::uint16_t     tag() const       { return read_word(0x00); }
	std::uint16_t     size() const      { return read_word(0x02); }
	void const *      data() const      { return m_buffer + 0x04; }
	ntfs_tag_reader   next() const      { return ntfs_tag_reader(m_buffer + total_length(), m_length - total_length()); }

	bool length_sufficient() const { return (m_length >= minimum_length()) && (m_length >= total_length()); }

	std::size_t total_length() const { return minimum_length() + size(); }
	static std::size_t minimum_length() { return 0x04; }

private:
	std::size_t m_length;
};


class ntfs_reader : private reader_base
{
public:
	ntfs_reader(extra_field_reader const &field) : reader_base(field.data()), m_length(field.data_size()) { }

	std::uint32_t   reserved() const    { return read_dword(0x00); }
	ntfs_tag_reader tag1() const        { return ntfs_tag_reader(m_buffer + 0x04, m_length - 4); }

	std::size_t total_length() const { return m_length; }
	static std::size_t minimum_length() { return 0x08; }

private:
	std::size_t m_length;
};


class ntfs_times_reader : private reader_base
{
public:
	ntfs_times_reader(ntfs_tag_reader const &tag) : reader_base(tag.data()) { }

	std::uint64_t   mtime() const   { return read_qword(0x00); }
	std::uint64_t   atime() const   { return read_qword(0x08); }
	std::uint64_t   ctime() const   { return read_qword(0x10); }

	std::size_t total_length() const { return minimum_length(); }
	static std::size_t minimum_length() { return 0x18; }
};


class general_flag_reader
{
public:
	general_flag_reader(std::uint16_t val) : m_value(val) { }

	bool        encrypted() const               { return bool(m_value & 0x0001); }
	bool        implode_8k_dict() const         { return bool(m_value & 0x0002); }
	bool        implode_3_trees() const         { return bool(m_value & 0x0004); }
	unsigned    deflate_option() const          { return unsigned((m_value >> 1) & 0x0003); }
	bool        lzma_eos_mark() const           { return bool(m_value & 0x0002); }
	bool        use_descriptor() const          { return bool(m_value & 0x0008); }
	bool        patch_data() const              { return bool(m_value & 0x0020); }
	bool        strong_encryption() const       { return bool(m_value & 0x0040); }
	bool        utf8_encoding() const           { return bool(m_value & 0x0800); }
	bool        directory_encryption() const    { return bool(m_value & 0x2000); }

private:
	std::uint16_t m_value;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

std::array<zip_file_impl::ptr, zip_file_impl::CACHE_SIZE> zip_file_impl::s_cache;
std::mutex zip_file_impl::s_cache_mutex;



/*-------------------------------------------------
    zip_file_close - close a ZIP file and add it
    to the cache
-------------------------------------------------*/

void zip_file_impl::close(ptr &&zip)
{
	if (!zip) return;

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


/***************************************************************************
    CONTAINED FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_search - return the next matching
    entry in the ZIP
-------------------------------------------------*/

int zip_file_impl::search(std::uint32_t search_crc, const std::string &search_filename, bool matchcrc, bool matchname, bool partialpath)
{
	// if we're at or past the end, we're done
	while ((m_cd_pos + central_dir_entry_reader::minimum_length()) <= m_ecd.cd_size)
	{
		// make sure we have enough data
		central_dir_entry_reader const reader(&m_cd[0] + m_cd_pos);
		if (!reader.signature_correct() || ((m_cd_pos + reader.total_length()) > m_ecd.cd_size))
			break;

		// extract file header info
		m_header.version_created     = reader.version_created();
		m_header.version_needed      = reader.version_needed();
		m_header.bit_flag            = reader.general_flag();
		m_header.compression         = reader.compression_method();
		m_header.crc                 = reader.crc32();
		m_header.compressed_length   = reader.compressed_size();
		m_header.uncompressed_length = reader.uncompressed_size();
		m_header.start_disk_number   = reader.start_disk();
		m_header.local_header_offset = reader.header_offset();

		// don't immediately decode DOS timestamp - it's expensive
		m_header.modified_date       = reader.modified_date();
		m_header.modified_time       = reader.modified_time();
		m_header.modified_cached     = false;

		// advance the position
		m_cd_pos += reader.total_length();

		// copy the filename
		bool is_utf8(general_flag_reader(m_header.bit_flag).utf8_encoding());
		reader.file_name(m_header.file_name);

		// walk the extra data
		for (auto extra = reader.extra_field(); extra.length_sufficient(); extra = extra.next())
		{
			// look for ZIP64 extended info
			if ((extra.header_id() == 0x0001) && (extra.data_size() >= zip64_ext_info_reader::minimum_length()))
			{
				zip64_ext_info_reader const ext64(reader, extra);
				if (extra.data_size() >= ext64.total_length())
				{
					m_header.compressed_length   = ext64.compressed_size();
					m_header.uncompressed_length = ext64.uncompressed_size();
					m_header.start_disk_number   = ext64.start_disk();
					m_header.local_header_offset = ext64.header_offset();
				}
			}

			// look for Info-ZIP UTF-8 path
			if (!is_utf8 && (extra.header_id() == 0x7075) && (extra.data_size() >= utf8_path_reader::minimum_length()))
			{
				utf8_path_reader const utf8path(extra);
				if (utf8path.version() == 1)
				{
					auto const addr(m_header.file_name.empty() ? nullptr : &m_header.file_name[0]);
					auto const length(m_header.file_name.empty() ? 0 : m_header.file_name.length() * sizeof(m_header.file_name[0]));
					auto const crc(crc32_creator::simple(addr, length));
					if (utf8path.name_crc32() == crc.m_raw)
					{
						utf8path.unicode_name(m_header.file_name);
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
						m_header.modified = system_clock_time_point_from_ntfs_duration(ticks);
						m_header.modified_cached = true;
					}
				}
			}
		}

		// FIXME: if (!is_utf8) convert filename to UTF8 (assume CP437 or something)

		// chop off trailing slash for directory entries
		bool const is_dir(!m_header.file_name.empty() && (m_header.file_name.back() == '/'));
		if (is_dir) m_header.file_name.resize(m_header.file_name.length() - 1);

		// check to see if it matches query
		bool const crcmatch(search_crc == m_header.crc);
		auto const partialoffset(m_header.file_name.length() - search_filename.length());
		bool const partialpossible((m_header.file_name.length() > search_filename.length()) && (m_header.file_name[partialoffset - 1] == '/'));
		const bool namematch(
				!core_stricmp(search_filename.c_str(), m_header.file_name.c_str()) ||
				(partialpath && partialpossible && !core_stricmp(search_filename.c_str(), m_header.file_name.c_str() + partialoffset)));

		bool const found = ((!matchcrc && !matchname) || !is_dir) && (!matchcrc || crcmatch) && (!matchname || namematch);
		if (found)
		{
			m_curr_is_dir = is_dir;
			return 0;
		}
	}
	return -1;
}


/*-------------------------------------------------
    zip_file_decompress - decompress a file
    from a ZIP into the target buffer
-------------------------------------------------*/

archive_file::error zip_file_impl::decompress(void *buffer, std::uint32_t length)
{
	archive_file::error ziperr;
	std::uint64_t offset;

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
	ziperr = get_compressed_data_offset(offset);
	if (ziperr != archive_file::error::NONE)
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

	case 14:
		ziperr = decompress_data_type_14(offset, buffer, length);
		break;

	default:
		osd_printf_error(
				"unzip: %s in %s uses unsupported compression method %u\n",
				m_header.file_name.c_str(), m_filename.c_str(), m_header.compression);
		ziperr = archive_file::error::UNSUPPORTED;
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

archive_file::error zip_file_impl::read_ecd()
{
	// make sure the file handle is open
	auto const ziperr = reopen();
	if (ziperr != archive_file::error::NONE)
		return ziperr;

	// we may need multiple tries
	osd_file::error error;
	std::uint32_t read_length;
	std::uint32_t buflen = 1024;
	while (buflen < 65536)
	{
		// max out the buffer length at the size of the file
		if (buflen > m_length)
			buflen = m_length;

		// allocate buffer
		std::unique_ptr<std::uint8_t []> buffer;
		try { buffer.reset(new std::uint8_t[buflen + 1]); }
		catch (...)
		{
			osd_printf_error("unzip: %s failed to allocate memory for ECD search\n", m_filename);
			return archive_file::error::OUT_OF_MEMORY;
		}

		// read in one buffers' worth of data
		error = m_file->read(&buffer[0], m_length - buflen, buflen, read_length);
		if ((error != osd_file::error::NONE) || (read_length != buflen))
		{
			osd_printf_error("unzip: %s error reading file to search for ECD (%d)\n", m_filename, int(error));
			return archive_file::error::FILE_ERROR;
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
			osd_printf_verbose("unzip: found %s ECD\n", m_filename);

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
				return archive_file::error::NONE;
			}

			// try to read the ZIP64 ECD locator
			error = m_file->read(
					&buffer[0],
					m_length - buflen + offset - ecd64_locator_reader::minimum_length(),
					ecd64_locator_reader::minimum_length(),
					read_length);
			if ((error != osd_file::error::NONE) || (read_length != ecd64_locator_reader::minimum_length()))
			{
				osd_printf_error("unzip: error reading %s to search for ZIP64 ECD locator (%d)\n", m_filename, int(error));
				return archive_file::error::FILE_ERROR;
			}

			// if the signature isn't correct, it's not a ZIP64 archive
			ecd64_locator_reader const ecd64_loc_rd(buffer.get());
			if (!ecd64_loc_rd.signature_correct())
			{
				osd_printf_verbose("unzip: %s has no ZIP64 ECD locator\n", m_filename);
				return archive_file::error::NONE;
			}

			// check that the ZIP64 ECD is in this segment (assuming this segment is the last segment)
			if ((ecd64_loc_rd.ecd64_disk() + 1) != ecd64_loc_rd.total_disks())
			{
				osd_printf_error("unzip: %s ZIP64 ECD resides in a different segment\n", m_filename);
				return archive_file::error::UNSUPPORTED;
			}

			// try to read the ZIP64 ECD
			error = m_file->read(&buffer[0], ecd64_loc_rd.ecd64_offset(), ecd64_reader::minimum_length(), read_length);
			if (error != osd_file::error::NONE)
			{
				osd_printf_error("unzip: error reading %s ZIP64 ECD (%d)\n", m_filename, int(error));
				return archive_file::error::FILE_ERROR;
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
						"unzip: %s ZIP64 ECD has incorrect signature 0x%08lx\n",
						m_filename.c_str(), long(ecd64_rd.signature()));
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
						m_filename.c_str(), unsigned(ecd64_rd.version_needed() / 10), unsigned(ecd64_rd.version_needed() % 10));
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

			return archive_file::error::NONE;
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
	return archive_file::error::OUT_OF_MEMORY;
}


/*-------------------------------------------------
    get_compressed_data_offset - return the
    offset of the compressed data
-------------------------------------------------*/

archive_file::error zip_file_impl::get_compressed_data_offset(std::uint64_t &offset)
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
				m_header.file_name.c_str(), m_filename.c_str(), unsigned(m_header.version_needed / 10), unsigned(m_header.version_needed % 10));
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
	if (ziperr != archive_file::error::NONE)
		return ziperr;

	// now go read the fixed-sized part of the local file header
	std::uint32_t read_length;
	auto const error = m_file->read(&m_buffer[0], m_header.local_header_offset, local_file_header_reader::minimum_length(), read_length);
	if (error != osd_file::error::NONE)
	{
		osd_printf_error(
				"unzip: error reading local file header for %s in %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(error));
		return archive_file::error::FILE_ERROR;
	}
	if (read_length != local_file_header_reader::minimum_length())
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading local file header for %s in %s\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::FILE_TRUNCATED;
	}

	// compute the final offset
	local_file_header_reader reader(&m_buffer[0]);
	if (!reader.signature_correct())
	{
		osd_printf_error(
				"unzip: local file header for %s in %s has incorrect signature %04lx\n",
				m_header.file_name.c_str(), m_filename.c_str(), long(reader.signature()));
		return archive_file::error::BAD_SIGNATURE;
	}
	offset = m_header.local_header_offset + reader.total_length();

	return archive_file::error::NONE;
}



/***************************************************************************
    DECOMPRESSION INTERFACES
***************************************************************************/

/*-------------------------------------------------
    decompress_data_type_0 - "decompress"
    type 0 data (which is uncompressed)
-------------------------------------------------*/

archive_file::error zip_file_impl::decompress_data_type_0(std::uint64_t offset, void *buffer, std::uint32_t length)
{
	// the data is uncompressed; just read it
	std::uint32_t read_length(0);
	auto const filerr = m_file->read(buffer, offset, m_header.compressed_length, read_length);
	if (filerr != osd_file::error::NONE)
	{
		osd_printf_error("unzip: error reading %s from %s (%d)\n", m_header.file_name, m_filename, int(filerr));
		return archive_file::error::FILE_ERROR;
	}
	else if (read_length != m_header.compressed_length)
	{
		osd_printf_error("unzip: unexpectedly reached end-of-file while reading %s from %s\n", m_header.file_name, m_filename);
		return archive_file::error::FILE_TRUNCATED;
	}
	else
	{
		return archive_file::error::NONE;
	}
}


/*-------------------------------------------------
    decompress_data_type_8 - decompress
    type 8 data (which is deflated)
-------------------------------------------------*/

archive_file::error zip_file_impl::decompress_data_type_8(std::uint64_t offset, void *buffer, std::uint32_t length)
{
	std::uint64_t input_remaining = m_header.compressed_length;
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
		osd_printf_error(
				"unzip: error allocating zlib stream to inflate %s from %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), zerr);
		return archive_file::error::DECOMPRESS_ERROR;
	}

	// loop until we're done
	while (true)
	{
		// read in the next chunk of data
		std::uint32_t read_length(0);
		auto const filerr = m_file->read(
				&m_buffer[0],
				offset,
				std::uint32_t((std::min<std::uint64_t>)(input_remaining, m_buffer.size())),
				read_length);
		if (filerr != osd_file::error::NONE)
		{
			osd_printf_error(
					"unzip: error reading compressed data for %s in %s (%d)\n",
					m_header.file_name.c_str(), m_filename.c_str(), int(filerr));
			inflateEnd(&stream);
			return archive_file::error::FILE_ERROR;
		}
		offset += read_length;

		// if we read nothing, but still have data left, the file is truncated
		if ((read_length == 0) && (input_remaining > 0))
		{
			osd_printf_error(
					"unzip: unexpectedly reached end-of-file while reading compressed data for %s in %s\n",
					m_header.file_name.c_str(), m_filename.c_str());
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
			osd_printf_error("unzip: error inflating %s from %s (%d)\n", m_header.file_name, m_filename, zerr);
			inflateEnd(&stream);
			return archive_file::error::DECOMPRESS_ERROR;
		}
	}

	// finish decompression
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
	{
		osd_printf_error("unzip: error finishing inflation of %s from %s (%d)\n", m_header.file_name, m_filename, zerr);
		return archive_file::error::DECOMPRESS_ERROR;
	}

	// if anything looks funny, report an error
	if ((stream.avail_out > 0) || (input_remaining > 0))
	{
		osd_printf_error(
				"unzip: inflation of %s from %s doesn't appear to have completed correctly\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::DECOMPRESS_ERROR;
	}

	return archive_file::error::NONE;
}


/*-------------------------------------------------
    decompress_data_type_14 - decompress
    type 14 data (LZMA)
-------------------------------------------------*/

archive_file::error zip_file_impl::decompress_data_type_14(std::uint64_t offset, void *buffer, std::uint32_t length)
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

	std::uint32_t read_length;
	osd_file::error filerr;
	SRes lzerr;
	ELzmaStatus lzstatus(LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK);

	// reset the stream
	ISzAlloc alloc_imp;
	alloc_imp.Alloc = [](void *p, std::size_t size) -> void * { return (size == 0) ? nullptr : std::malloc(size); };
	alloc_imp.Free = [](void *p, void *address) -> void { std::free(address); };
	CLzmaDec stream;
	LzmaDec_Construct(&stream);

	// need to read LZMA properties before we can initialise the decompressor
	if (4 > input_remaining)
	{
		osd_printf_error(
				"unzip:compressed data for %s in %s is too small to hold LZMA properties header\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::DECOMPRESS_ERROR;
	}
	filerr = m_file->read(&m_buffer[0], offset, 4, read_length);
	if (filerr != osd_file::error::NONE)
	{
		osd_printf_error(
				"unzip: error reading LZMA properties header for %s in %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(filerr));
		return archive_file::error::FILE_ERROR;
	}
	offset += read_length;
	input_remaining -= read_length;
	if (4 != read_length)
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading LZMA properties header for %s in %s\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::FILE_TRUNCATED;
	}
	std::uint16_t const props_size((std::uint16_t(m_buffer[3]) << 8) | std::uint16_t(m_buffer[2]));
	if (props_size > m_buffer.size())
	{
		osd_printf_error(
				"unzip: %s in %s has excessively large LZMA properties\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::UNSUPPORTED;
	}
	else if (props_size > input_remaining)
	{
		osd_printf_error(
				"unzip:compressed data for %s in %s is too small to hold LZMA properties\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::DECOMPRESS_ERROR;
	}
	filerr = m_file->read(&m_buffer[0], offset, props_size, read_length);
	if (filerr != osd_file::error::NONE)
	{
		osd_printf_error(
				"unzip: error reading LZMA properties for %s in %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(filerr));
		return archive_file::error::FILE_ERROR;
	}
	offset += read_length;
	input_remaining -= read_length;
	if (props_size != read_length)
	{
		osd_printf_error(
				"unzip: unexpectedly reached end-of-file while reading LZMA properties for %s in %s\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::FILE_TRUNCATED;
	}

	// initialize the decompressor
	lzerr = LzmaDec_Allocate(&stream, &m_buffer[0], props_size, &alloc_imp);
	if (SZ_ERROR_MEM == lzerr)
	{
		osd_printf_error(
				"unzip: memory error allocating LZMA decoder to decompress %s from %s\n",
				m_header.file_name.c_str(), m_filename.c_str());
		return archive_file::error::OUT_OF_MEMORY;
	}
	else if (SZ_OK != lzerr)
	{
		osd_printf_error(
				"unzip: error allocating LZMA decoder to decompress %s from %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(lzerr));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	LzmaDec_Init(&stream);

	// loop until we're done
	while (0 < input_remaining)
	{
		// read in the next chunk of data
		filerr = m_file->read(
				&m_buffer[0],
				offset,
				std::uint32_t((std::min<std::uint64_t>)(input_remaining, m_buffer.size())),
				read_length);
		if (filerr != osd_file::error::NONE)
		{
			osd_printf_error(
					"unzip: error reading compressed data for %s in %s (%d)\n",
					m_header.file_name.c_str(), m_filename.c_str(), int(filerr));
			LzmaDec_Free(&stream, &alloc_imp);
			return archive_file::error::FILE_ERROR;
		}
		offset += read_length;
		input_remaining -= read_length;

		// if we read nothing, but still have data left, the file is truncated
		if ((0 == read_length) && (0 < input_remaining))
		{
			osd_printf_error(
					"unzip: unexpectedly reached end-of-file while reading compressed data for %s in %s\n",
					m_header.file_name.c_str(), m_filename.c_str());
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
				((0 == input_remaining) && eos_mark) ? LZMA_FINISH_END :  LZMA_FINISH_ANY,
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
		return archive_file::error::NONE;
	}
	else if (eos_mark)
	{
		osd_printf_error(
				"unzip: LZMA end mark not found for %s in %s (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(lzstatus));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	else if (LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK != lzstatus)
	{
		osd_printf_error(
				"unzip: LZMA decompression of %s from %s doesn't appear to have completed correctly (%d)\n",
				m_header.file_name.c_str(), m_filename.c_str(), int(lzstatus));
		return archive_file::error::DECOMPRESS_ERROR;
	}
	else
	{
		return archive_file::error::NONE;
	}
}

} // anonymous namespace



/***************************************************************************
    un7z.cpp TRAMPOLINES
***************************************************************************/

void m7z_file_cache_clear();



/***************************************************************************
    ZIP FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_open - opens a ZIP file for reading
-------------------------------------------------*/

archive_file::error archive_file::open_zip(const std::string &filename, ptr &result)
{
	// ensure we start with a nullptr result
	result.reset();

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
		result = std::make_unique<zip_file_wrapper>(std::move(newimpl));
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

void archive_file::cache_clear()
{
	zip_file_impl::cache_clear();
	m7z_file_cache_clear();
}


archive_file::~archive_file()
{
}

} // namespace util
