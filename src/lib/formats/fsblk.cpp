// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem operations on mounted image blocks

#include "fsblk.h"

#include "multibyte.h"
#include "strformat.h"

#include <cstring>
#include <stdexcept>

namespace fs {

void refcounted_inner::ref()
{
	m_ref ++;
}

void refcounted_inner::ref_weak()
{
	m_weak_ref ++;
}

bool refcounted_inner::unref()
{
	m_ref --;
	if(m_ref == 0) {
		if(m_weak_ref)
			drop_weak_references();
		else
			delete this;
		return true;
	}
	return false;
}

bool refcounted_inner::unref_weak()
{
	m_weak_ref --;
	if(m_weak_ref == 0 && m_ref == 0) {
		delete this;
		return true;
	}
	return false;
}



void fsblk_t::set_block_size(u32 block_size)
{
	m_block_size = block_size;
}

u8 *fsblk_t::iblock_t::offset(const char *function, u32 off, u32 size)
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
	return data() + off;
}

const u8 *fsblk_t::iblock_t::rooffset(const char *function, u32 off, u32 size)
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
	return rodata() + off;
}

void fsblk_t::block_t::copy(u32 offset, const u8 *src, u32 size)
{
	memcpy(m_object->offset("copy", offset, size), src, size);
}

void fsblk_t::block_t::fill(u32 offset, u8 data, u32 size)
{
	memset(m_object->offset("fill", offset, size), data, size);
}

void fsblk_t::block_t::fill(u8 data)
{
	memset(m_object->data(), data, m_object->size());
}

void fsblk_t::block_t::wstr(u32 offset, std::string_view str)
{
	memcpy(m_object->offset("wstr", offset, str.size()), str.data(), str.size());
}

void fsblk_t::block_t::w8(u32 offset, u8 data)
{
	m_object->offset("w8", offset, 1)[0] = data;
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	put_u16be(m_object->offset("w16b", offset, 2), data);
}

void fsblk_t::block_t::w24b(u32 offset, u32 data)
{
	put_u24be(m_object->offset("w24b", offset, 3), data);
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	put_u32be(m_object->offset("w32b", offset, 4), data);
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	put_u16le(m_object->offset("w16l", offset, 2), data);
}

void fsblk_t::block_t::w24l(u32 offset, u32 data)
{
	put_u24le(m_object->offset("w24l", offset, 3), data);
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	put_u32le(m_object->offset("w32l", offset, 4), data);
}

std::string_view fsblk_t::block_t::rstr(u32 offset, u32 size) const
{
	const u8 *d = m_object->rooffset("rstr", offset, size);
	return std::string_view(reinterpret_cast<const char *>(d), size);
}

u8 fsblk_t::block_t::r8(u32 offset) const
{
	return m_object->offset("r8", offset, 1)[0];
}

u16 fsblk_t::block_t::r16b(u32 offset) const
{
	return get_u16be(m_object->offset("r16b", offset, 2));
}

u32 fsblk_t::block_t::r24b(u32 offset) const
{
	return get_u24be(m_object->offset("r24b", offset, 3));
}

u32 fsblk_t::block_t::r32b(u32 offset) const
{
	return get_u32be(m_object->offset("r32b", offset, 4));
}

u16 fsblk_t::block_t::r16l(u32 offset) const
{
	return get_u16le(m_object->offset("r16l", offset, 2));
}

u32 fsblk_t::block_t::r24l(u32 offset) const
{
	return get_u24le(m_object->offset("r24l", offset, 3));
}

u32 fsblk_t::block_t::r32l(u32 offset) const
{
	return get_u32le(m_object->offset("r32l", offset, 4));
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

err_t filesystem_t::volume_metadata_change(const meta_data &meta)
{
	return ERR_UNSUPPORTED;
}

std::pair<err_t, meta_data> filesystem_t::metadata(const std::vector<std::string> &path)
{
	return std::make_pair(ERR_UNSUPPORTED, meta_data());
}

err_t filesystem_t::metadata_change(const std::vector<std::string> &path, const meta_data &meta)
{
	return ERR_UNSUPPORTED;
}

std::pair<err_t, std::vector<dir_entry>> filesystem_t::directory_contents(const std::vector<std::string> &path)
{
	return std::make_pair(ERR_UNSUPPORTED, std::vector<dir_entry>());
}

err_t filesystem_t::rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath)
{
	return ERR_UNSUPPORTED;
}

err_t filesystem_t::remove(const std::vector<std::string> &path)
{
	return ERR_UNSUPPORTED;
}

err_t filesystem_t::dir_create(const std::vector<std::string> &path, const meta_data &meta)
{
	return ERR_UNSUPPORTED;
}

err_t filesystem_t::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	return ERR_UNSUPPORTED;
}

std::pair<err_t, std::vector<u8>> filesystem_t::file_read(const std::vector<std::string> &path)
{
	return std::make_pair(ERR_UNSUPPORTED, std::vector<u8>());
}

err_t filesystem_t::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	return ERR_UNSUPPORTED;
}

std::pair<err_t, std::vector<u8>> filesystem_t::file_rsrc_read(const std::vector<std::string> &path)
{
	return std::make_pair(ERR_UNSUPPORTED, std::vector<u8>());
}

err_t filesystem_t::file_rsrc_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	return ERR_UNSUPPORTED;
}

err_t filesystem_t::format(const meta_data &meta)
{
	return ERR_UNSUPPORTED;
}

} // namespace fs
