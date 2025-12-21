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

const u8 *fsblk_t::block_t::roffs(const char *function, u32 off, u32 size) const
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
	return rodata() + off;
}

u8 *fsblk_t::block_t::woffs(const char *function, u32 off, u32 size)
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
	return data() + off;
}

void fsblk_t::block_t::write(u32 offset, const u8 *src, u32 size)
{
	memcpy(woffs("write", offset, size), src, size);
}

void fsblk_t::block_t::fill(u32 offset, u8 data, u32 size)
{
	memset(woffs("fill", offset, size), data, size);
}

void fsblk_t::block_t::fill(u8 data)
{
	memset(this->data(), data, size());
}

void fsblk_t::block_t::wstr(u32 offset, std::string_view str)
{
	memcpy(woffs("wstr", offset, str.size()), str.data(), str.size());
}

void fsblk_t::block_t::w8(u32 offset, u8 data)
{
	woffs("w8", offset, 1)[0] = data;
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	put_u16be(woffs("w16b", offset, 2), data);
}

void fsblk_t::block_t::w24b(u32 offset, u32 data)
{
	put_u24be(woffs("w24b", offset, 3), data);
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	put_u32be(woffs("w32b", offset, 4), data);
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	put_u16le(woffs("w16l", offset, 2), data);
}

void fsblk_t::block_t::w24l(u32 offset, u32 data)
{
	put_u24le(woffs("w24l", offset, 3), data);
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	put_u32le(woffs("w32l", offset, 4), data);
}

void fsblk_t::block_t::read(u32 offset, u8 *dst, u32 size) const
{
	memcpy(dst, roffs("read", offset, size), size);
}

std::string_view fsblk_t::block_t::rstr(u32 offset, u32 size) const
{
	const u8 *d = roffs("rstr", offset, size);
	return std::string_view(reinterpret_cast<const char *>(d), size);
}

u8 fsblk_t::block_t::r8(u32 offset) const
{
	return roffs("r8", offset, 1)[0];
}

u16 fsblk_t::block_t::r16b(u32 offset) const
{
	return get_u16be(roffs("r16b", offset, 2));
}

u32 fsblk_t::block_t::r24b(u32 offset) const
{
	return get_u24be(roffs("r24b", offset, 3));
}

u32 fsblk_t::block_t::r32b(u32 offset) const
{
	return get_u32be(roffs("r32b", offset, 4));
}

u16 fsblk_t::block_t::r16l(u32 offset) const
{
	return get_u16le(roffs("r16l", offset, 2));
}

u32 fsblk_t::block_t::r24l(u32 offset) const
{
	return get_u24le(roffs("r24l", offset, 3));
}

u32 fsblk_t::block_t::r32l(u32 offset) const
{
	return get_u32le(roffs("r32l", offset, 4));
}



void filesystem_t::wstr(u8 *p, std::string_view str)
{
	memcpy(p, str.data(), str.size());
}

std::string_view filesystem_t::rstr(const u8 *p, u32 size)
{
	return std::string_view(reinterpret_cast<const char *>(p), size);
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
