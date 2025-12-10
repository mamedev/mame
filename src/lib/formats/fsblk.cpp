// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem operations on mounted image blocks

#include "fsblk.h"

#include "multibyte.h"
#include "strformat.h"

#include <cstring>
#include <stdexcept>

namespace fs {

void fsblk_t::set_block_size(u32 block_size)
{
	m_block_size = block_size;
}

void fsblk_t::block_t::rcheck(const char *function, u32 off, u32 size) const
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
}

void fsblk_t::block_t::wcheck(const char *function, u32 off, u32 size) const
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
}

void fsblk_t::block_t::write(u32 offset, const u8 *src, u32 size)
{
	wcheck("write", offset, size);
	internal_write(offset, src, size);
}

void fsblk_t::block_t::fill(u32 offset, u8 data, u32 size)
{
	wcheck("fill", offset, size);
	internal_fill(offset, data, size);
}

void fsblk_t::block_t::fill(u8 data)
{
	internal_fill(0, data, size());
}

void fsblk_t::block_t::wstr(u32 offset, std::string_view str)
{
	wcheck("wstr", offset, str.size());
	internal_write(offset, reinterpret_cast<const u8 *>(str.data()), str.size());
}

void fsblk_t::block_t::w8(u32 offset, u8 data)
{
	wcheck("w8", offset, 1);
	internal_fill(offset, data, 1);
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	wcheck("w16b", offset, 2);
	u8 p[2];
	put_u16be(p, data);
	internal_write(offset, p, 2);
}

void fsblk_t::block_t::w24b(u32 offset, u32 data)
{
	wcheck("w24b", offset, 3);
	u8 p[3];
	put_u24be(p, data);
	internal_write(offset, p, 3);
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	wcheck("w32b", offset, 4);
	u8 p[4];
	put_u32be(p, data);
	internal_write(offset, p, 4);
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	wcheck("w16l", offset, 2);
	u8 p[2];
	put_u16le(p, data);
	internal_write(offset, p, 2);
}

void fsblk_t::block_t::w24l(u32 offset, u32 data)
{
	wcheck("w24l", offset, 3);
	u8 p[3];
	put_u24le(p, data);
	internal_write(offset, p, 3);
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	wcheck("w32l", offset, 4);
	u8 p[4];
	put_u32le(p, data);
	internal_write(offset, p, 4);
}

void fsblk_t::block_t::read(u32 offset, u8 *dst, u32 size) const
{
	rcheck("read", offset, size);
	internal_read(offset, dst, size);
}

std::string fsblk_t::block_t::rstr(u32 offset, u32 size) const
{
	rcheck("rstr", offset, size);
	std::string s(size, 0);
	internal_read(offset, reinterpret_cast<u8 *>(s.data()), size);
	return s;
}

u8 fsblk_t::block_t::r8(u32 offset) const
{
	rcheck("r8", offset, 1);
	u8 p;
	internal_read(offset, &p, 1);
	return p;
}

u16 fsblk_t::block_t::r16b(u32 offset) const
{
	rcheck("r16b", offset, 2);
	u8 p[2];
	internal_read(offset, p, 2);
	return get_u16be(p);
}

u32 fsblk_t::block_t::r24b(u32 offset) const
{
	rcheck("r24b", offset, 3);
	u8 p[3];
	internal_read(offset, p, 3);
	return get_u24be(p);
}

u32 fsblk_t::block_t::r32b(u32 offset) const
{
	rcheck("r32b", offset, 4);
	u8 p[4];
	internal_read(offset, p, 4);
	return get_u32be(p);
}

u16 fsblk_t::block_t::r16l(u32 offset) const
{
	rcheck("r16l", offset, 2);
	u8 p[2];
	internal_read(offset, p, 2);
	return get_u16le(p);
}

u32 fsblk_t::block_t::r24l(u32 offset) const
{
	rcheck("r24l", offset, 3);
	u8 p[3];
	internal_read(offset, p, 3);
	return get_u24le(p);
}

u32 fsblk_t::block_t::r32l(u32 offset) const
{
	rcheck("r32l", offset, 4);
	u8 p[4];
	internal_read(offset, p, 4);
	return get_u32le(p);
}

bool fsblk_t::block_t::eqmem(u32 offset, const u8 *p, u32 size) const
{
	rcheck("eqmem", offset, size);
	return internal_eqmem(offset, p, size);
}

bool fsblk_t::block_t::eqstr(u32 offset, std::string_view str) const
{
	rcheck("eqstr", offset, str.size());
	return internal_eqmem(offset, reinterpret_cast<const u8 *>(str.data()), str.size());
}


void filesystem_t::wstr(u8 *p, std::string_view str)
{
	memcpy(p, str.data(), str.size());
}

std::string filesystem_t::rstr(const u8 *p, u32 size)
{
	return std::string(reinterpret_cast<const char *>(p), size);
}

std::string_view filesystem_t::trim_end_spaces(std::string_view str)
{
	const auto i = str.find_last_not_of(' ');
	return str.substr(0, (std::string::npos != i) ? (i + 1) : 0);
}

meta_data filesystem_t::volume_metadata()
{
	return meta_data();
}

std::error_condition filesystem_t::volume_metadata_change(const meta_data &meta)
{
	return error::unsupported;
}

std::pair<std::error_condition, meta_data> filesystem_t::metadata(const std::vector<std::string> &path)
{
	return std::make_pair(error::unsupported, meta_data());
}

std::error_condition filesystem_t::metadata_change(const std::vector<std::string> &path, const meta_data &meta)
{
	return error::unsupported;
}

std::pair<std::error_condition, std::vector<dir_entry>> filesystem_t::directory_contents(const std::vector<std::string> &path)
{
	return std::make_pair(error::unsupported, std::vector<dir_entry>());
}

std::error_condition filesystem_t::rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath)
{
	return error::unsupported;
}

std::error_condition filesystem_t::remove(const std::vector<std::string> &path)
{
	return error::unsupported;
}

std::error_condition filesystem_t::dir_create(const std::vector<std::string> &path, const meta_data &meta)
{
	return error::unsupported;
}

std::error_condition filesystem_t::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	return error::unsupported;
}

std::pair<std::error_condition, std::vector<u8>> filesystem_t::file_read(const std::vector<std::string> &path)
{
	return std::make_pair(error::unsupported, std::vector<u8>());
}

std::error_condition filesystem_t::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	return error::unsupported;
}

std::pair<std::error_condition, std::vector<u8>> filesystem_t::file_rsrc_read(const std::vector<std::string> &path)
{
	return std::make_pair(error::unsupported, std::vector<u8>());
}

std::error_condition filesystem_t::file_rsrc_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	return error::unsupported;
}

std::error_condition filesystem_t::format(const meta_data &meta)
{
	return error::unsupported;
}

std::error_category const &fs_category() noexcept
{
	class fs_category_impl : public std::error_category
	{
	public:
		virtual char const *name() const noexcept override { return "fs"; }

		virtual std::string message(int condition) const override
		{
			using namespace std::literals;
			static std::string_view const s_messages[] = {
					"No error"sv,
					"Unsupported operation"sv,
					"File or directory not found"sv,
					"No space on volume"sv,
					"Invalid block number"sv,
					"Invalid filename or path"sv,
					"Incorrect file size"sv,
					"File already exists"sv,
					"Circular reference"sv,
			};
			if ((0 <= condition) && (std::size(s_messages) > condition))
				return std::string(s_messages[condition]);
			else
				return "Unknown error"s;
		}
	};
	static fs_category_impl const s_fs_category_instance;
	return s_fs_category_instance;
}

} // namespace fs
