// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    archiver.cpp

    Archive writer implementation for ZIP archives

    TODO:
    * Allow control of method/compression level.
    * Report better error codes.
    * Support archive members 4 GiB and larger.
    * Produces technically invalid archives if the output is not seekable
      and the total archive size is over 4 GiB.
    * Produces invalid archives if a member is smaller than 4 GiB but the
      compression algorithm produces larger output of 4 GiB or more.

***************************************************************************/

#include "archiver.h"

#include "hashing.h"
#include "multibyte.h"
#include "pkzipdefs.h"
#include "timeconv.h"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <functional>
#include <limits>
#include <tuple>
#include <string>
#include <type_traits>
#include <vector>


namespace util {

namespace {

class pkzip_writer_impl final : public archive_writer, private pkzip_defs
{
private:
	struct entry_info
	{
		entry_info() = default;
		entry_info(entry_info const &) = default;
		entry_info(entry_info &&) = default;
		entry_info& operator=(entry_info const &) = default;
		entry_info& operator=(entry_info &&) = default;

		std::uint64_t offset;
		std::uint64_t raw_size;
		std::uint64_t compressed_size;
		std::chrono::system_clock::time_point modification_time;
		std::string name;
		std::string comment;

		std::uint32_t crc;
		std::uint16_t method;
		std::uint16_t dos_time;
		std::uint16_t dos_date;
	};

	class member_writer final : public write_stream
	{
	private:
		pkzip_writer_impl &m_archive;
		bool m_active;

	public:
		using ptr = std::unique_ptr<member_writer>;

		member_writer(pkzip_writer_impl &archive) : m_archive(archive), m_active(true)
		{
		}

		~member_writer()
		{
			if (m_active)
				finalize();
		}

		virtual std::error_condition finalize() noexcept override
		{
			// can't write if finalised, failed, etc.
			if (!m_active)
				return std::errc::invalid_argument;

			// pad with zeroes if smaller than declared size
			if (m_archive.m_current_size && (*m_archive.m_current_size > m_archive.m_entries.back().raw_size))
			{
				std::uint64_t remain(*m_archive.m_current_size - m_archive.m_entries.back().raw_size);
				std::uint8_t zeroes[512];
				std::fill(std::begin(zeroes), std::end(zeroes), 0);
				while (remain)
				{
					auto const chunk(std::min<std::common_type_t<std::size_t, std::uint64_t> >(std::size(zeroes), remain));
					std::size_t actual;
					auto const err = write_some(zeroes, std::size_t(chunk), actual);
					remain -= actual;
					if (err)
						return err;
				}
			}

			// take necessary actions for the selected method
			switch (m_archive.m_entries.back().method)
			{
			case METHOD_STORE:
				break;
			case METHOD_DEFLATE:
				{
					auto const err = m_archive.deflate_finalize();
					if (err)
						return err;
				}
				break;
			default:
				// shouldn't get here
				return std::errc::invalid_argument;
			}

			// write out sizes and checksum
			m_active = false;
			return m_archive.finalize_member();
		}

		virtual std::error_condition flush() noexcept override
		{
			// doesn't make sense if finalised, failed, etc.
			if (!m_active)
				return std::errc::invalid_argument;

			// take necessary actions for the selected method
			switch (m_archive.m_entries.back().method)
			{
			case METHOD_STORE:
				return std::error_condition();
			case METHOD_DEFLATE:
				return m_archive.deflate_flush_buffer();
			default:
				// shouldn't get here
				return std::errc::invalid_argument;
			}
		}

		virtual std::error_condition write_some(
				void const *buffer,
				std::size_t length,
				std::size_t &actual) noexcept override
		{
			// can't write if finalised, failed, etc.
			if (!m_active)
			{
				actual = 0;
				return std::errc::invalid_argument;
			}

			// don't allow writing past declared size
			if (m_archive.m_current_size && ((m_archive.m_entries.back().raw_size + length) > *m_archive.m_current_size))
			{
				actual = 0;
				return std::errc::invalid_argument;
			}

			// limited ZIP64 support for now
			if (std::numeric_limits<std::uint32_t>::max() < (m_archive.m_entries.back().raw_size + length))
			{
				actual = 0;
				return std::errc::invalid_argument;
			}

			// take action based on method
			switch (m_archive.m_entries.back().method)
			{
			case METHOD_STORE:
				return m_archive.store_some(buffer, length, actual);
			case METHOD_DEFLATE:
				return m_archive.deflate_some(buffer, length, actual);
			}

			// shouldn't get here
			actual = 0;
			return std::errc::invalid_argument;
		}

		void enter_failed()
		{
			m_active = false;
		}
	};

	using entry_info_vector = std::vector<entry_info>;

	entry_info_vector m_entries;
	write_stream::ptr m_owned;
	std::optional<std::reference_wrapper<write_stream> > m_output;
	std::optional<std::reference_wrapper<random_write> > m_random;
	std::int64_t m_logical_offset;
	std::uint64_t m_offset;

	std::optional<std::reference_wrapper<member_writer> > m_current_member;
	std::optional<std::uint64_t> m_current_size;
	crc32_creator m_current_crc;

	z_stream m_z_stream;
	bool m_z_inited, m_z_finished;

	std::uint8_t m_buffer[8 * 1024];

	void set_up(
			std::optional<std::uint64_t> physical_offset,
			std::optional<std::uint64_t> logical_offset)
	{
		if (m_random)
		{
			std::error_condition err;
			if (physical_offset)
			{
				m_offset = *physical_offset;
				err = random().seek(m_offset, SEEK_SET);
			}
			else
			{
				err = random().tell(m_offset);
			}
			if (err)
				throw err;
			if (!logical_offset)
				m_logical_offset = 0;
			else
				m_logical_offset = *logical_offset - m_offset;
		}
		else
		{
			assert(!physical_offset);
			assert(logical_offset);

			m_logical_offset = *logical_offset;
			m_offset = 0;
		}

		m_z_stream.zalloc = Z_NULL;
		m_z_stream.zfree = Z_NULL;
		m_z_stream.opaque = Z_NULL;
		m_z_inited = m_z_finished = false;
	}

	write_stream &output() noexcept
	{
		assert(m_output);
		return m_output->get();
	}

	random_write &random() noexcept
	{
		assert(m_random);
		return m_random->get();
	}

	member_writer &current_member() noexcept
	{
		assert(m_current_member);
		return m_current_member->get();
	}

	void enter_failed() noexcept
	{
		m_entries.clear();
		m_owned.reset();
		m_output = std::nullopt;
		m_random = std::nullopt;
		if (m_current_member)
		{
			current_member().enter_failed();
			m_current_member = std::nullopt;
		}
	}

	std::error_condition store_some(
			void const *buffer,
			std::size_t length,
			std::size_t &actual) noexcept
	{
		std::error_condition err(output().write_some(buffer, length, actual));
		if (actual)
		{
			m_offset += actual;
			m_current_crc.append(buffer, actual);
			m_entries.back().raw_size += actual;
			m_entries.back().compressed_size += actual;
		}
		return err;
	}

	std::error_condition deflate_some(
			void const *buffer,
			std::size_t length,
			std::size_t &actual) noexcept
	{
		actual = 0U;

		auto ptr(reinterpret_cast<Bytef const *>(buffer));
		while (length)
		{
			m_z_stream.next_in = ptr;
			m_z_stream.avail_in = uInt(std::min<std::common_type_t<uInt, std::size_t> >(std::numeric_limits<uInt>::max(), length));

			bool const complete(m_current_size && ((m_entries.back().raw_size + m_z_stream.avail_in) >= *m_current_size));
			auto const zerr = deflate(&m_z_stream, complete ? Z_FINISH : Z_NO_FLUSH);
			if (Z_STREAM_END == zerr)
			{
				m_z_finished = true;
			}
			else if (Z_OK != zerr)
			{
				enter_failed();
				return std::errc::io_error;
			}
			auto const consumed(m_z_stream.next_in - ptr);
			m_current_crc.append(ptr, consumed);
			m_entries.back().raw_size += consumed;

			if (!m_z_stream.avail_out)
			{
				auto const err = deflate_flush_buffer();
				if (err)
					return err;
			}

			ptr += consumed;
			length -= consumed;
			actual += consumed;
			assert(!length || !m_z_finished);
		}

		return std::error_condition();
	}

	std::error_condition deflate_finalize() noexcept
	{
		if (!m_z_finished)
		{
			m_z_stream.next_in = nullptr;
			m_z_stream.avail_in = 0;
			while (true)
			{
				auto const zerr = deflate(&m_z_stream, Z_FINISH);
				if ((Z_STREAM_END != zerr) && (Z_OK != zerr))
				{
					enter_failed();
					return std::errc::io_error;
				}

				if (Z_STREAM_END == zerr)
					break;

				if (!m_z_stream.avail_out)
				{
					auto const err = deflate_flush_buffer();
					if (err)
						return err;
				}
			}
		}
		return deflate_flush_buffer();
	}

	std::error_condition finalize_member() noexcept
	{
		// FIXME: there's a small chance of getting here when raw size is under 4GiB but compressed size isn't
		std::error_condition err;
		std::size_t written;
		m_entries.back().crc = m_current_crc.finish();
		if (m_random)
		{
			// fill in CRC and sizes in local file header
			put_u32le(&m_buffer[OFFS_LCL_HDR_CRC32 - OFFS_LCL_HDR_CRC32], m_entries.back().crc);
			put_u32le(&m_buffer[OFFS_LCL_HDR_CMP_SIZE - OFFS_LCL_HDR_CRC32], m_entries.back().compressed_size);
			put_u32le(&m_buffer[OFFS_LCL_HDR_RAW_SIZE - OFFS_LCL_HDR_CRC32], m_entries.back().raw_size);
			std::tie(err, written) = write_at(
					random(),
					m_entries.back().offset + OFFS_LCL_HDR_CRC32,
					m_buffer,
					OFFS_LCL_HDR_NAME_LEN - OFFS_LCL_HDR_CRC32);
			assert(err || ((OFFS_LCL_HDR_NAME_LEN - OFFS_LCL_HDR_CRC32) == written));
		}
		else
		{
			// append a data descriptor
			put_u32le(&m_buffer[OFFS_DATA_DESC_SIG], SIG_DATA_DESC);
			put_u32le(&m_buffer[OFFS_DATA_DESC_CRC32], m_entries.back().crc);
			put_u32le(&m_buffer[OFFS_DATA_DESC_CMP_SIZE], m_entries.back().compressed_size);
			put_u32le(&m_buffer[OFFS_DATA_DESC_RAW_SIZE], m_entries.back().raw_size);
			std::tie(err, written) = write(output(), m_buffer, OFFS_DATA_DESC_RAW_SIZE + sizeof(std::uint32_t));
			assert(err || (OFFS_DATA_DESC_RAW_SIZE + sizeof(std::uint32_t) == written));
			m_offset += written;
		}
		if (err)
		{
			enter_failed();
			return err;
		}

		// clean up current member
		m_current_member = std::nullopt;
		m_current_size = std::nullopt;
		m_current_crc.reset();

		return std::error_condition();
	}

	std::error_condition deflate_flush_buffer() noexcept
	{
		auto const produced(std::size(m_buffer) - m_z_stream.avail_out);
		if (!produced)
			return std::error_condition();

		auto const [err, written] = write(output(), m_buffer, produced);
		m_offset += written;
		m_entries.back().compressed_size += written;
		if (!err)
		{
			m_z_stream.next_out = reinterpret_cast<Bytef *>(&m_buffer[0]);
			m_z_stream.avail_out = std::size(m_buffer);
		}
		else
		{
			enter_failed();
		}
		return err;
	}

public:
	pkzip_writer_impl(
			random_write::ptr &&output,
			std::optional<std::uint64_t> physical_offset,
			std::optional<std::uint64_t> logical_offset)
	{
		assert(output);
		m_output = *output;
		m_random = *output;
		m_owned = std::move(output);
		set_up(physical_offset, logical_offset);
	}

	pkzip_writer_impl(
			random_write &output,
			std::optional<std::uint64_t> physical_offset,
			std::optional<std::uint64_t> logical_offset)
	{
		m_output = output;
		m_random = output;
		set_up(physical_offset, logical_offset);
	}

	pkzip_writer_impl(write_stream::ptr &&output, std::uint64_t offset)
	{
		assert(output);
		m_output = *output;
		m_owned = std::move(output);
		set_up(std::nullopt, offset);
	}

	pkzip_writer_impl(write_stream &output, std::uint64_t offset)
	{
		m_output = output;
		set_up(std::nullopt, offset);
	}

	~pkzip_writer_impl()
	{
		// deleting the archive with an open member is a bad idea, but at least try not to crash
		if (m_current_member)
			current_member().finalize();

		if (m_z_inited)
			deflateEnd(&m_z_stream);
	}

	virtual std::pair<write_stream::ptr, std::error_condition> add_file(
			std::string_view name,
			std::chrono::system_clock::time_point modification_time,
			std::optional<std::uint64_t> size,
			std::optional<std::string_view> comment) noexcept override
	{
		// can't create a member if finalised, failed, already writing a member, etc.
		if (!m_output || m_current_member)
			return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);

		// limited ZIP64 support for now
		if (size && (std::numeric_limits<std::uint32_t>::max() < *size))
			return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);

		// sanity-check the name
		if (!name.empty()) 
		{
			if ((name.front() == '/') || (name.back() == '/') || (name.find("//") != std::string_view::npos))
				return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);
		}

		// calculate pessimistic central directory entry size - must be no larger than 64 KiB
		std::uint64_t cd_header_size(OFFS_CD_HDR_NAME);
		cd_header_size += name.length();
		if (comment)
			cd_header_size += comment->length();
		cd_header_size += 28; // ZIP64 sizes and offset
		cd_header_size += 36; // NTFS timestamps
		if ((64 * 1024) < cd_header_size)
			return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);

		// check that the modification time can be represented in DOS format
		std::time_t mtime(std::chrono::system_clock::to_time_t(modification_time));
		std::tm const *pltime(std::localtime(&mtime));
		if (!pltime)
			return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);
		std::tm const local_time(*pltime);
		if ((80 > local_time.tm_year) || ((0x7f + 80) < local_time.tm_year))
			return std::make_pair(write_stream::ptr(), std::errc::invalid_argument);

		// choose a compression method
		std::uint16_t method;
		if (size && (128 > size))
			method = METHOD_STORE;
		else
			method = METHOD_DEFLATE;

		// initialise zlib compression if necessary
		if (METHOD_DEFLATE == method)
		{
			int zerr;
			if (m_z_inited)
				zerr = deflateReset(&m_z_stream);
			else
				zerr = deflateInit2(&m_z_stream, 9, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
			if (Z_OK != zerr)
				return std::make_pair(write_stream::ptr(), std::errc::not_enough_memory);
			m_z_stream.next_out = reinterpret_cast<Bytef *>(&m_buffer[0]);
			m_z_stream.avail_out = std::size(m_buffer);
			m_z_inited = true;
			m_z_finished = false;
		}

		// allocate entry info
		member_writer::ptr member;
		try
		{
			member.reset(new member_writer(*this));

			std::string name_copy(name);
			std::string comment_copy;
			if (comment)
				comment_copy = *comment;
			entry_info &entry(m_entries.emplace_back());
			entry.offset = m_offset;
			entry.raw_size = 0;
			entry.compressed_size = 0;
			entry.modification_time = modification_time;
			entry.name = std::move(name_copy);
			entry.comment = std::move(comment_copy);

			entry.method = method;
			entry.dos_time = (local_time.tm_hour << 11) | (local_time.tm_min << 5) | (local_time.tm_sec >> 1);
			entry.dos_date = ((local_time.tm_year - 80) << 9) | ((local_time.tm_mon + 1) << 5) | local_time.tm_mday;
		}
		catch (std::bad_alloc const &)
		{
			return std::make_pair(write_stream::ptr(), std::errc::not_enough_memory);
		}

		// prepare a local file header
		uint8_t versreq;
		if (METHOD_DEFLATE == method)
			versreq = 20;
		else
			versreq = 10;
		uint16_t gpflag(0);
		if (METHOD_DEFLATE == method)
			gpflag |= GP_FLAG_DEFLATE_MAX;
		if (!m_random)
			gpflag |= GP_FLAG_DATA_DESC;
		gpflag |= GP_FLAG_UTF8;
		put_u32le(&m_buffer[OFFS_LCL_HDR_SIG], SIG_LCL_HDR);
		m_buffer[OFFS_LCL_HDR_VER_REQ] = versreq;
		m_buffer[OFFS_LCL_HDR_VER_REQ + 1] = 0x00; // just say attributes are DOS FAT format
		put_u16le(&m_buffer[OFFS_LCL_HDR_GP_FLAG], gpflag);
		put_u16le(&m_buffer[OFFS_LCL_HDR_METHOD], method);
		put_u16le(&m_buffer[OFFS_LCL_HDR_MOD_TIME], m_entries.back().dos_time);
		put_u16le(&m_buffer[OFFS_LCL_HDR_MOD_DATE], m_entries.back().dos_date);
		put_u32le(&m_buffer[OFFS_LCL_HDR_CRC32], 0U);
		put_u32le(&m_buffer[OFFS_LCL_HDR_CMP_SIZE], 0U);
		put_u32le(&m_buffer[OFFS_LCL_HDR_RAW_SIZE], 0U);
		put_u16le(&m_buffer[OFFS_LCL_HDR_NAME_LEN], name.length());
		put_u16le(&m_buffer[OFFS_LCL_HDR_EXTRA_LEN], 0U);

		// try to write the local file header and name
		std::error_condition err;
		std::size_t written;
		std::tie(err, written) = write(output(), m_buffer, OFFS_LCL_HDR_NAME);
		m_offset += written;
		if (err)
		{
			m_entries.pop_back();
			if (written)
				enter_failed();
			return std::make_pair(write_stream::ptr(), err);
		}
		if (!name.empty())
		{
			std::tie(err, written) = write(output(), name.data(), name.length());
			m_offset += written;
			if (err)
			{
				enter_failed();
				return std::make_pair(write_stream::ptr(), err);
			}
		}

		m_current_member = *member;
		m_current_size = size;
		m_current_crc.reset();
		return std::make_pair(std::move(member), std::error_condition());
	}

	virtual std::error_condition add_folder(
			std::string_view name,
			std::chrono::system_clock::time_point modification_time,
			std::optional<std::string_view> comment) noexcept override
	{
		// can't create a member if finalised, failed, already writing a member, etc.
		if (!m_output || m_current_member)
			return std::errc::invalid_argument;

		// sanity-check the name
		if (name.empty() || (name.front() == '/') || (name.find("//") != std::string_view::npos))
			return std::errc::invalid_argument;

		// calculate pessimistic central directory entry size - must be no larger than 64 KiB
		std::uint64_t cd_header_size(OFFS_CD_HDR_NAME);
		cd_header_size += name.length();
		if (comment)
			cd_header_size += comment->length();
		cd_header_size += 12; // ZIP64 offset
		cd_header_size += 36; // NTFS timestamps
		if ((64 * 1024) < cd_header_size)
			return std::errc::invalid_argument;

		// check that the modification time can be represented in DOS format
		std::time_t mtime(std::chrono::system_clock::to_time_t(modification_time));
		std::tm const *pltime(std::localtime(&mtime));
		if (!pltime)
			return std::errc::invalid_argument;
		std::tm const local_time(*pltime);
		if ((80 > local_time.tm_year) || ((0x7f + 80) < local_time.tm_year))
			return std::errc::invalid_argument;

		// allocate entry info
		try
		{
			std::string name_copy;
			if (name.back() != '/')
				name_copy.reserve(name.length() + 1);
			name_copy = name;
			if (name.back() != '/')
				name_copy.append(1, '/');
			std::string comment_copy;
			if (comment)
				comment_copy = *comment;
			entry_info &entry(m_entries.emplace_back());
			entry.offset = m_offset;
			entry.raw_size = 0;
			entry.compressed_size = 0;
			entry.modification_time = modification_time;
			entry.name = std::move(name_copy);
			entry.comment = std::move(comment_copy);

			entry.method = 0;
			entry.dos_time = (local_time.tm_hour << 11) | (local_time.tm_min << 5) | (local_time.tm_sec >> 1);
			entry.dos_date = ((local_time.tm_year - 80) << 9) | ((local_time.tm_mon + 1) << 5) | local_time.tm_mday;
		}
		catch (std::bad_alloc const &)
		{
			return std::errc::not_enough_memory;
		}

		// prepare a local file header
		put_u32le(&m_buffer[OFFS_LCL_HDR_SIG], SIG_LCL_HDR);
		m_buffer[OFFS_LCL_HDR_VER_REQ] = 20;
		m_buffer[OFFS_LCL_HDR_VER_REQ + 1] = 0x00; // just say attributes are DOS FAT format
		put_u16le(&m_buffer[OFFS_LCL_HDR_GP_FLAG], GP_FLAG_UTF8);
		put_u16le(&m_buffer[OFFS_LCL_HDR_METHOD], METHOD_STORE);
		put_u16le(&m_buffer[OFFS_LCL_HDR_MOD_TIME], m_entries.back().dos_time);
		put_u16le(&m_buffer[OFFS_LCL_HDR_MOD_DATE], m_entries.back().dos_date);
		put_u32le(&m_buffer[OFFS_LCL_HDR_CRC32], 0U);
		put_u32le(&m_buffer[OFFS_LCL_HDR_CMP_SIZE], 0U);
		put_u32le(&m_buffer[OFFS_LCL_HDR_RAW_SIZE], 0U);
		put_u16le(&m_buffer[OFFS_LCL_HDR_NAME_LEN], m_entries.back().name.length());
		put_u16le(&m_buffer[OFFS_LCL_HDR_EXTRA_LEN], 0U);

		// try to write the local file header and name
		std::error_condition err;
		std::size_t written;
		std::tie(err, written) = write(output(), m_buffer, OFFS_LCL_HDR_NAME);
		m_offset += written;
		if (err)
		{
			m_entries.pop_back();
			if (written)
				enter_failed();
			return err;
		}
		std::tie(err, written) = write(output(), m_entries.back().name.data(), m_entries.back().name.length());
		m_offset += written;
		if (err)
			enter_failed();
		return err;
	}

	virtual std::error_condition finalize() noexcept override
	{
		if (!m_output || m_current_member)
			return std::errc::invalid_argument;

		std::error_condition err;

		std::uint64_t cd_offset(m_offset);
		std::uint8_t *const end(m_buffer + std::size(m_buffer));
		std::uint8_t *ptr(m_buffer);
		auto const flush_buffered =
				[this, &ptr] ()
				{
					auto const [err, written] = write(output(), m_buffer, ptr - m_buffer);
					m_offset += written;
					ptr = std::copy(m_buffer + written, ptr, m_buffer);
					if (err)
						enter_failed();
					return err;
				};
		auto const append_string =
				[this, &ptr, &end, &flush_buffered] (auto const &str) -> std::error_condition
				{
					if (((end - ptr) >= str.length()) && (str.length() < 1024))
					{
						std::memcpy(ptr, str.data(), str.length());
						ptr += str.length();
						return std::error_condition();
					}

					auto const err = flush_buffered();
					if (err)
						return err;
					if (str.length() < 512)
					{
						assert((end - ptr) >= str.length());
						std::memcpy(ptr, str.data(), str.length());
						ptr += str.length();
						return std::error_condition();
					}

					auto const [ferr, written] = write(output(), str.data(), str.length());
					m_offset += written;
					if (ferr)
						enter_failed();
					return ferr;
				};

		bool need_zip64(false);
		need_zip64 = need_zip64 || (std::numeric_limits<std::uint32_t>::max() < cd_offset);
		need_zip64 = need_zip64 || (std::numeric_limits<std::uint16_t>::max() < m_entries.size());
		for (std::size_t i = 0; !need_zip64 && (m_entries.size() > i); ++i)
		{
			need_zip64 = need_zip64 || (std::numeric_limits<std::uint32_t>::max() < m_entries[i].offset);
			need_zip64 = need_zip64 || (std::numeric_limits<std::uint32_t>::max() < m_entries[i].raw_size);
			need_zip64 = need_zip64 || (std::numeric_limits<std::uint32_t>::max() < m_entries[i].compressed_size);
		}

		for (std::size_t i = 0; m_entries.size() > i; ++i)
		{
			auto const &entry(m_entries[i]);

			auto const ntfstime = ntfs_duration_from_system_clock_time_point(entry.modification_time).count();

			std::size_t extra64_size(0);
			if (need_zip64)
			{
				if (std::numeric_limits<std::uint32_t>::max() <= entry.offset)
					extra64_size += 8;
				if (std::numeric_limits<std::uint32_t>::max() <= entry.raw_size)
					extra64_size += 8;
				if (std::numeric_limits<std::uint32_t>::max() <= entry.compressed_size)
					extra64_size += 8;
			}

			std::uint16_t const extra_length = 36 + (extra64_size ? (extra64_size + 4) : 0);

			std::uint16_t const versreq(extra64_size ? 45 : ((METHOD_DEFLATE == entry.method) || ('/' == entry.name.back())) ? 20 : 10);

			std::uint16_t gpflag(0);
			if (METHOD_DEFLATE == entry.method)
				gpflag |= GP_FLAG_DEFLATE_MAX;
			gpflag |= GP_FLAG_UTF8;

			if (OFFS_CD_HDR_NAME < (end - ptr))
			{
				err = flush_buffered();
				if (err)
					return err;
			}

			assert((ptr + OFFS_CD_HDR_NAME) <= end);
			put_u32le(&ptr[OFFS_CD_HDR_SIG], SIG_CD_HDR);
			put_u16le(&ptr[OFFS_CD_HDR_VER_CREATE], 45);
			put_u16le(&ptr[OFFS_CD_HDR_VER_REQ], versreq);
			put_u16le(&ptr[OFFS_CD_HDR_GP_FLAG], gpflag);
			put_u16le(&ptr[OFFS_CD_HDR_METHOD], entry.method);
			put_u16le(&ptr[OFFS_CD_HDR_MOD_TIME], entry.dos_time);
			put_u16le(&ptr[OFFS_CD_HDR_MOD_DATE], entry.dos_date);
			put_u32le(&ptr[OFFS_CD_HDR_CRC32], entry.crc);
			put_u32le(&ptr[OFFS_CD_HDR_CMP_SIZE], std::min<uint64_t>(std::numeric_limits<std::uint32_t>::max(), entry.compressed_size));
			put_u32le(&ptr[OFFS_CD_HDR_RAW_SIZE], std::min<uint64_t>(std::numeric_limits<std::uint32_t>::max(), entry.raw_size));
			put_u16le(&ptr[OFFS_CD_HDR_NAME_LEN], entry.name.length());
			put_u16le(&ptr[OFFS_CD_HDR_EXTRA_LEN], extra_length);
			put_u16le(&ptr[OFFS_CD_HDR_CMT_LEN], entry.comment.length());
			put_u16le(&ptr[OFFS_CD_HDR_DISK], 0);
			put_u16le(&ptr[OFFS_CD_HDR_INT_ATTR], 0);
			put_u32le(&ptr[OFFS_CD_HDR_EXT_ATTR], ('/' == entry.name.back()) ? EXT_ATTR_FAT_SUBDIR : 0U);
			put_u32le(&ptr[OFFS_CD_HDR_OFFS], std::min<uint64_t>(std::numeric_limits<std::uint32_t>::max(), entry.offset));
			ptr += OFFS_CD_HDR_NAME;

			err = append_string(entry.name);
			if (err)
				return err;

			if (extra64_size)
			{
				if ((end - ptr) < (extra64_size + 4))
				{
					err = flush_buffered();
					if (err)
						return err;
				}

				assert((ptr + extra64_size + 4) <= end);
				put_u16le(&ptr[0], EXTRA_ID_ZIP64);
				put_u16le(&ptr[2], extra64_size);
				ptr += 4;
				if (std::numeric_limits<std::uint32_t>::max() <= entry.raw_size)
				{
					put_u64le(ptr, entry.raw_size);
					ptr += 8;
				}
				if (std::numeric_limits<std::uint32_t>::max() <= entry.compressed_size)
				{
					put_u64le(ptr, entry.compressed_size);
					ptr += 8;
				}
				if (std::numeric_limits<std::uint32_t>::max() <= entry.offset)
				{
					put_u64le(ptr, entry.offset);
					ptr += 8;
				}
			}

			if ((end - ptr) < 36)
			{
				err = flush_buffered();
				if (err)
					return err;
			}

			assert((ptr + 36) <= end);
			put_u16le(&ptr[0], EXTRA_ID_NTFS);
			put_u16le(&ptr[2], 32);
			put_u32le(&ptr[4], 0);
			put_u16le(&ptr[8], EXTRA_NTFS_TAG_TIMES);
			put_u16le(&ptr[10], 24);
			put_u64le(&ptr[12], ntfstime);
			put_u64le(&ptr[20], ntfstime);
			put_u64le(&ptr[28], ntfstime);
			ptr += 36;

			err = append_string(entry.comment);
			if (err)
				return err;
		}

		std::uint64_t const cd_size(m_offset + (ptr - m_buffer) - cd_offset);
		need_zip64 = need_zip64 || (std::numeric_limits<std::uint32_t>::max() < cd_size);

		if (need_zip64)
		{
			std::uint64_t const ecd64_offset(m_offset + (ptr - m_buffer));

			if ((OFFS_ECD64_EXT_DATA + OFFS_ECD64_LOC_DISKS + sizeof(std::uint32_t)) < (end - ptr))
			{
				err = flush_buffered();
				if (err)
					return err;
			}

			assert((ptr + OFFS_ECD64_EXT_DATA) <= end);
			put_u32le(&ptr[OFFS_ECD64_SIG], SIG_ECD64);
			put_u64le(&ptr[OFFS_ECD64_SIZE], OFFS_ECD64_EXT_DATA - OFFS_ECD64_VER_CREATE);
			put_u16le(&ptr[OFFS_ECD64_VER_CREATE], 45);
			put_u16le(&ptr[OFFS_ECD64_VER_REQ], 45);
			put_u32le(&ptr[OFFS_ECD64_THIS_DISK], 0);
			put_u32le(&ptr[OFFS_ECD64_CD_START_DISK], 0);
			put_u64le(&ptr[OFFS_ECD64_THIS_ENTRIES], m_entries.size());
			put_u64le(&ptr[OFFS_ECD64_CD_ENTRIES], m_entries.size());
			put_u64le(&ptr[OFFS_ECD64_CD_SIZE], cd_size);
			put_u64le(&ptr[OFFS_ECD64_CD_OFFS], cd_offset);
			ptr += OFFS_ECD64_EXT_DATA;

			assert((ptr + OFFS_ECD64_LOC_DISKS + sizeof(std::uint32_t)) <= end);
			put_u32le(&ptr[OFFS_ECD64_LOC_SIG], SIG_ECD64_LOC);
			put_u32le(&ptr[OFFS_ECD64_LOC_ECD64_DISK], 0);
			put_u64le(&ptr[OFFS_ECD64_LOC_ECD64_OFFS], ecd64_offset);
			put_u32le(&ptr[OFFS_ECD64_LOC_DISKS], 1);
			ptr += OFFS_ECD64_LOC_DISKS + sizeof(std::uint32_t);
		}

		if (OFFS_ECD_CMT < (end - ptr))
		{
			err = flush_buffered();
			if (err)
				return err;
		}

		assert((ptr + OFFS_ECD_CMT) <= end);
		put_u32le(&ptr[OFFS_ECD_SIG], SIG_ECD);
		put_u16le(&ptr[OFFS_ECD_THIS_DISK], 0);
		put_u16le(&ptr[OFFS_ECD_CD_START_DISK], 0);
		put_u16le(&ptr[OFFS_ECD_THIS_ENTRIES], std::min<std::size_t>(std::numeric_limits<std::uint16_t>::max(), m_entries.size()));
		put_u16le(&ptr[OFFS_ECD_CD_ENTRIES], std::min<std::size_t>(std::numeric_limits<std::uint16_t>::max(), m_entries.size()));
		put_u32le(&ptr[OFFS_ECD_CD_SIZE], std::min<std::uint64_t>(std::numeric_limits<std::uint32_t>::max(), cd_size));
		put_u32le(&ptr[OFFS_ECD_CD_OFFS], std::min<std::uint64_t>(std::numeric_limits<std::uint32_t>::max(), cd_offset));
		put_u16le(&ptr[OFFS_ECD_CMT_LEN], 0);
		ptr += OFFS_ECD_CMT;

		if (ptr != m_buffer)
		{
			err = flush_buffered();
			if (err)
				return err;
		}

		if (m_owned)
		{
			err = output().finalize();
			if (err)
			{
				enter_failed();
				return err;
			}
		}
		err = output().flush();
		m_owned.reset();
		m_output = std::nullopt;
		m_random = std::nullopt;
		return err;
	}
};

} // anonymous namespace


std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		random_write::ptr &&output,
		std::optional<std::uint64_t> physical_offset,
		std::optional<std::uint64_t> logical_offset) noexcept
{
	try
	{
		if (!output)
			return std::make_pair(archive_writer::ptr(), std::errc::invalid_argument);
		return std::make_pair(
				std::make_unique<pkzip_writer_impl>(std::move(output), physical_offset, logical_offset),
				std::error_condition());
	}
	catch (std::error_condition const &err)
	{
		return std::make_pair(archive_writer::ptr(), err);
	}
	catch (std::bad_alloc const &)
	{
		return std::make_pair(archive_writer::ptr(), std::errc::not_enough_memory);
	}
}


std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		random_write &output,
		std::optional<std::uint64_t> physical_offset,
		std::optional<std::uint64_t> logical_offset) noexcept
{
	try
	{
		return std::make_pair(
				std::make_unique<pkzip_writer_impl>(output, physical_offset, logical_offset),
				std::error_condition());
	}
	catch (std::error_condition const &err)
	{
		return std::make_pair(archive_writer::ptr(), err);
	}
	catch (std::bad_alloc const &)
	{
		return std::make_pair(archive_writer::ptr(), std::errc::not_enough_memory);
	}
}


std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		write_stream::ptr &&output,
		std::uint64_t offset) noexcept
{
	try
	{
		if (!output)
			return std::make_pair(archive_writer::ptr(), std::errc::invalid_argument);
		return std::make_pair(
				std::make_unique<pkzip_writer_impl>(std::move(output), offset),
				std::error_condition());
	}
	catch (std::error_condition const &err)
	{
		return std::make_pair(archive_writer::ptr(), err);
	}
	catch (std::bad_alloc const &)
	{
		return std::make_pair(archive_writer::ptr(), std::errc::not_enough_memory);
	}
}


std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		write_stream &output,
		std::uint64_t offset) noexcept
{
	try
	{
		return std::make_pair(
				std::make_unique<pkzip_writer_impl>(output, offset),
				std::error_condition());
	}
	catch (std::error_condition const &err)
	{
		return std::make_pair(archive_writer::ptr(), err);
	}
	catch (std::bad_alloc const &)
	{
		return std::make_pair(archive_writer::ptr(), std::errc::not_enough_memory);
	}
}

} // namespace util
