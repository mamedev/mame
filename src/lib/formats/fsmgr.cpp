// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "fsmgr.h"

#include "strformat.h"

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



void manager_t::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
}

void manager_t::enumerate_h(hd_enumerator &he) const
{
}

void manager_t::enumerate_c(cdrom_enumerator &ce) const
{
}

bool manager_t::has_variant(const std::vector<u32> &variants, u32 variant)
{
	for(u32 v : variants)
		if(variant == v)
			return true;
	return false;
}

bool manager_t::has(u32 form_factor, const std::vector<u32> &variants, u32 ff, u32 variant)
{
	if(form_factor == floppy_image::FF_UNKNOWN)
		return true;
	if(form_factor != ff)
		return false;
	for(u32 v : variants)
		if(variant == v)
			return true;
	return false;
}

std::vector<meta_description> manager_t::volume_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

std::vector<meta_description> manager_t::file_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

std::vector<meta_description> manager_t::directory_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

char manager_t::directory_separator() const
{
	return 0; // Subdirectories not supported by default
}

void filesystem_t::format(const meta_data &meta)
{
	throw std::logic_error("format called on a filesystem not supporting it");
}

filesystem_t::dir_t filesystem_t::root()
{
	throw std::logic_error("root called on a filesystem not supporting it");
}

meta_data filesystem_t::metadata()
{
	throw std::logic_error("filesystem_t::metadata called on a filesystem not supporting it");
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
		throw std::out_of_range(util::string_format("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size));
	return rodata() + off;
}

void fsblk_t::block_t::copy(u32 offset, const u8 *src, u32 size)
{
	u8 *blk = m_object->offset("copy", offset, size);
	memcpy(blk, src, size);
}

void fsblk_t::block_t::fill(u32 offset, u8 data, u32 size)
{
	u8 *blk = m_object->offset("fill", offset, size);
	memset(blk, data, size);
}

void fsblk_t::block_t::fill(u8 data)
{
	u8 *blk = m_object->data();
	memset(blk, data, m_object->size());
}

void fsblk_t::block_t::wstr(u32 offset, const std::string &str)
{
	u8 *blk = m_object->offset("wstr", offset, str.size());
	memcpy(blk, str.data(), str.size());
}

void fsblk_t::block_t::w8(u32 offset, u8 data)
{
	u8 *blk = m_object->offset("w8", offset, 1);
	blk[0] = data;
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	u8 *blk = m_object->offset("w16b", offset, 2);
	blk[0] = data >> 8;
	blk[1] = data;
}

void fsblk_t::block_t::w24b(u32 offset, u32 data)
{
	u8 *blk = m_object->offset("w24b", offset, 3);
	blk[0] = data >> 16;
	blk[1] = data >> 8;
	blk[2] = data;
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	u8 *blk = m_object->offset("w32b", offset, 4);
	blk[0] = data >> 24;
	blk[1] = data >> 16;
	blk[2] = data >> 8;
	blk[3] = data;
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	u8 *blk = m_object->offset("w16l", offset, 2);
	blk[0] = data;
	blk[1] = data >> 8;
}

void fsblk_t::block_t::w24l(u32 offset, u32 data)
{
	u8 *blk = m_object->offset("w24l", offset, 3);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	u8 *blk = m_object->offset("w32l", offset, 4);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
	blk[3] = data >> 24;
}

std::string fsblk_t::block_t::rstr(u32 offset, u32 size) const
{
	const u8 *d = m_object->rooffset("rstr", offset, size);
	return std::string(d, d + size);
}

u8 fsblk_t::block_t::r8(u32 offset) const
{
	const u8 *blk = m_object->offset("r8", offset, 1);
	return blk[0];
}

u16 fsblk_t::block_t::r16b(u32 offset) const
{
	const u8 *blk = m_object->offset("r16b", offset, 2);
	return (blk[0] << 8) | blk[1];
}

u32 fsblk_t::block_t::r24b(u32 offset) const
{
	const u8 *blk = m_object->offset("r24b", offset, 3);
	return (blk[0] << 16) | (blk[1] << 8) | blk[2];
}

u32 fsblk_t::block_t::r32b(u32 offset) const
{
	const u8 *blk = m_object->offset("r32b", offset, 4);
	return (blk[0] << 24) | (blk[1] << 16) | (blk[2] << 8) | blk[3];
}

u16 fsblk_t::block_t::r16l(u32 offset) const
{
	const u8 *blk = m_object->offset("r16l", offset, 2);
	return blk[0] | (blk[1] << 8);
}

u32 fsblk_t::block_t::r24l(u32 offset) const
{
	const u8 *blk = m_object->offset("r24l", offset, 3);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16);
}

u32 fsblk_t::block_t::r32l(u32 offset) const
{
	const u8 *blk = m_object->offset("r32l", offset, 4);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16) | (blk[3] << 24);
}



void filesystem_t::copy(u8 *p, const u8 *src, u32 size)
{
	memcpy(p, src, size);
}

void filesystem_t::fill(u8 *p, u8 data, u32 size)
{
	memset(p, data, size);
}

void filesystem_t::wstr(u8 *p, const std::string &str)
{
	memcpy(p, str.data(), str.size());
}

void filesystem_t::w8(u8 *p, u8 data)
{
	p[0] = data;
}

void filesystem_t::w16b(u8 *p, u16 data)
{
	p[0] = data >> 8;
	p[1] = data;
}

void filesystem_t::w24b(u8 *p, u32 data)
{
	p[0] = data >> 16;
	p[1] = data >> 8;
	p[2] = data;
}

void filesystem_t::w32b(u8 *p, u32 data)
{
	p[0] = data >> 24;
	p[1] = data >> 16;
	p[2] = data >> 8;
	p[3] = data;
}

void filesystem_t::w16l(u8 *p, u16 data)
{
	p[0] = data;
	p[1] = data >> 8;
}

void filesystem_t::w24l(u8 *p, u32 data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
}

void filesystem_t::w32l(u8 *p, u32 data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
	p[3] = data >> 24;
}

std::string filesystem_t::rstr(const u8 *p, u32 size)
{
	return std::string(p, p + size);
}

u8 filesystem_t::r8(const u8 *p)
{
	return p[0];
}

u16 filesystem_t::r16b(const u8 *p)
{
	return (p[0] << 8) | p[1];
}

u32 filesystem_t::r24b(const u8 *p)
{
	return (p[0] << 16) | (p[1] << 8) | p[2];
}

u32 filesystem_t::r32b(const u8 *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

u16 filesystem_t::r16l(const u8 *p)
{
	return p[0] | (p[1] << 8);
}

u32 filesystem_t::r24l(const u8 *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16);
}

u32 filesystem_t::r32l(const u8 *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

std::string filesystem_t::trim_end_spaces(const std::string &str)
{
	const auto i = str.find_last_not_of(' ');
	return str.substr(0, (std::string::npos != i) ? (i + 1) : 0);
}

filesystem_t::file_t filesystem_t::idir_t::file_create(const meta_data &info)
{
	throw std::logic_error("file_create called on a filesystem not supporting write");
}

void filesystem_t::idir_t::file_delete(uint64_t key)
{
	throw std::logic_error("file_delete called on a filesystem not supporting write");
}


void filesystem_t::ifile_t::replace(const std::vector<u8> &data)
{
	throw std::logic_error("replace called on a filesystem not supporting write");
}

void filesystem_t::ifile_t::rsrc_replace(const std::vector<u8> &data)
{
	throw std::logic_error("rsrc_replace called on a filesystem not supporting write or resource forks");
}

void filesystem_t::ifile_t::metadata_change(const meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

void filesystem_t::idir_t::metadata_change(const meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

void filesystem_t::metadata_change(const meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

std::vector<u8> filesystem_t::ifile_t::rsrc_read_all()
{
	throw std::logic_error("rsrc_read_all called on a filesystem without resource forks");
}

} // namespace fs
