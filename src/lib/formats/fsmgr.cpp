// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "fsmgr.h"

#include "strformat.h"

#include <stdexcept>

void fs_refcounted_inner::ref()
{
	m_ref ++;
}

void fs_refcounted_inner::ref_weak()
{
	m_weak_ref ++;
}

bool fs_refcounted_inner::unref()
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

bool fs_refcounted_inner::unref_weak()
{
	m_weak_ref --;
	if(m_weak_ref == 0 && m_ref == 0) {
		delete this;
		return true;
	}
	return false;
}



void filesystem_manager_t::enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
}

void filesystem_manager_t::enumerate_h(hd_enumerator &he) const
{
}

void filesystem_manager_t::enumerate_c(cdrom_enumerator &ce) const
{
}

bool filesystem_manager_t::has_variant(const std::vector<uint32_t> &variants, uint32_t variant)
{
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

bool filesystem_manager_t::has(uint32_t form_factor, const std::vector<uint32_t> &variants, uint32_t ff, uint32_t variant)
{
	if(form_factor == floppy_image::FF_UNKNOWN)
		return true;
	if(form_factor != ff)
		return false;
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

std::vector<fs_meta_description> filesystem_manager_t::volume_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

std::vector<fs_meta_description> filesystem_manager_t::file_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

std::vector<fs_meta_description> filesystem_manager_t::directory_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

char filesystem_manager_t::directory_separator() const
{
	return 0; // Subdirectories not supported by default
}

void filesystem_t::format(const fs_meta_data &meta)
{
	throw std::logic_error("format called on a filesystem not supporting it");
}

filesystem_t::dir_t filesystem_t::root()
{
	throw std::logic_error("root called on a filesystem not supporting it");
}

fs_meta_data filesystem_t::metadata()
{
	throw std::logic_error("filesystem_t::metadata called on a filesystem not supporting it");
}

void fsblk_t::set_block_size(uint32_t block_size)
{
	m_block_size = block_size;
}


uint8_t *fsblk_t::iblock_t::offset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d", function, off, size, m_size));
	return data() + off;
}

const uint8_t *fsblk_t::iblock_t::rooffset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		throw std::out_of_range(util::string_format("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size));
	return rodata() + off;
}

void fsblk_t::block_t::copy(uint32_t offset, const uint8_t *src, uint32_t size)
{
	uint8_t *blk = m_object->offset("copy", offset, size);
	memcpy(blk, src, size);
}

void fsblk_t::block_t::fill(uint32_t offset, uint8_t data, uint32_t size)
{
	uint8_t *blk = m_object->offset("fill", offset, size);
	memset(blk, data, size);
}

void fsblk_t::block_t::fill(uint8_t data)
{
	uint8_t *blk = m_object->data();
	memset(blk, data, m_object->size());
}

void fsblk_t::block_t::wstr(uint32_t offset, const std::string &str)
{
	uint8_t *blk = m_object->offset("wstr", offset, str.size());
	memcpy(blk, str.data(), str.size());
}

void fsblk_t::block_t::w8(uint32_t offset, uint8_t data)
{
	uint8_t *blk = m_object->offset("w8", offset, 1);
	blk[0] = data;
}

void fsblk_t::block_t::w16b(uint32_t offset, uint16_t data)
{
	uint8_t *blk = m_object->offset("w16b", offset, 2);
	blk[0] = data >> 8;
	blk[1] = data;
}

void fsblk_t::block_t::w24b(uint32_t offset, uint32_t data)
{
	uint8_t *blk = m_object->offset("w24b", offset, 3);
	blk[0] = data >> 16;
	blk[1] = data >> 8;
	blk[2] = data;
}

void fsblk_t::block_t::w32b(uint32_t offset, uint32_t data)
{
	uint8_t *blk = m_object->offset("w32b", offset, 4);
	blk[0] = data >> 24;
	blk[1] = data >> 16;
	blk[2] = data >> 8;
	blk[3] = data;
}

void fsblk_t::block_t::w16l(uint32_t offset, uint16_t data)
{
	uint8_t *blk = m_object->offset("w16l", offset, 2);
	blk[0] = data;
	blk[1] = data >> 8;
}

void fsblk_t::block_t::w24l(uint32_t offset, uint32_t data)
{
	uint8_t *blk = m_object->offset("w24l", offset, 3);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
}

void fsblk_t::block_t::w32l(uint32_t offset, uint32_t data)
{
	uint8_t *blk = m_object->offset("w32l", offset, 4);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
	blk[3] = data >> 24;
}

std::string fsblk_t::block_t::rstr(uint32_t offset, uint32_t size)
{
	const uint8_t *d = m_object->rooffset("rstr", offset, size);
	return std::string(d, d + size);
}

uint8_t fsblk_t::block_t::r8(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r8", offset, 1);
	return blk[0];
}

uint16_t fsblk_t::block_t::r16b(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r16b", offset, 2);
	return (blk[0] << 8) | blk[1];
}

uint32_t fsblk_t::block_t::r24b(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r24b", offset, 3);
	return (blk[0] << 16) | (blk[1] << 8) | blk[2];
}

uint32_t fsblk_t::block_t::r32b(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r32b", offset, 4);
	return (blk[0] << 24) | (blk[1] << 16) | (blk[2] << 8) | blk[3];
}

uint16_t fsblk_t::block_t::r16l(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r16l", offset, 2);
	return blk[0] | (blk[1] << 8);
}

uint32_t fsblk_t::block_t::r24l(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r24l", offset, 3);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16);
}

uint32_t fsblk_t::block_t::r32l(uint32_t offset)
{
	const uint8_t *blk = m_object->offset("r32l", offset, 4);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16) | (blk[3] << 24);
}



void filesystem_t::copy(uint8_t *p, const uint8_t *src, uint32_t size)
{
	memcpy(p, src, size);
}

void filesystem_t::fill(uint8_t *p, uint8_t data, uint32_t size)
{
	memset(p, data, size);
}

void filesystem_t::wstr(uint8_t *p, const std::string &str)
{
	memcpy(p, str.data(), str.size());
}

void filesystem_t::w8(uint8_t *p, uint8_t data)
{
	p[0] = data;
}

void filesystem_t::w16b(uint8_t *p, uint16_t data)
{
	p[0] = data >> 8;
	p[1] = data;
}

void filesystem_t::w24b(uint8_t *p, uint32_t data)
{
	p[0] = data >> 16;
	p[1] = data >> 8;
	p[2] = data;
}

void filesystem_t::w32b(uint8_t *p, uint32_t data)
{
	p[0] = data >> 24;
	p[1] = data >> 16;
	p[2] = data >> 8;
	p[3] = data;
}

void filesystem_t::w16l(uint8_t *p, uint16_t data)
{
	p[0] = data;
	p[1] = data >> 8;
}

void filesystem_t::w24l(uint8_t *p, uint32_t data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
}

void filesystem_t::w32l(uint8_t *p, uint32_t data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
	p[3] = data >> 24;
}

std::string filesystem_t::rstr(const uint8_t *p, uint32_t size)
{
	return std::string(p, p + size);
}

uint8_t filesystem_t::r8(const uint8_t *p)
{
	return p[0];
}

uint16_t filesystem_t::r16b(const uint8_t *p)
{
	return (p[0] << 8) | p[1];
}

uint32_t filesystem_t::r24b(const uint8_t *p)
{
	return (p[0] << 16) | (p[1] << 8) | p[2];
}

uint32_t filesystem_t::r32b(const uint8_t *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

uint16_t filesystem_t::r16l(const uint8_t *p)
{
	return p[0] | (p[1] << 8);
}

uint32_t filesystem_t::r24l(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16);
}

uint32_t filesystem_t::r32l(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

std::string filesystem_t::trim_end_spaces(const std::string &str)
{
	const auto i = str.find_last_not_of(' ');
	return str.substr(0, (std::string::npos != i) ? (i + 1) : 0);
}

filesystem_t::file_t filesystem_t::idir_t::file_create(const fs_meta_data &info)
{
	throw std::logic_error("file_create called on a filesystem not supporting write");
}

void filesystem_t::idir_t::file_delete(uint64_t key)
{
	throw std::logic_error("file_delete called on a filesystem not supporting write");
}


void filesystem_t::ifile_t::replace(const std::vector<uint8_t> &data)
{
	throw std::logic_error("replace called on a filesystem not supporting write");
}

void filesystem_t::ifile_t::rsrc_replace(const std::vector<uint8_t> &data)
{
	throw std::logic_error("rsrc_replace called on a filesystem not supporting write or resource forks");
}

void filesystem_t::ifile_t::metadata_change(const fs_meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

void filesystem_t::idir_t::metadata_change(const fs_meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

void filesystem_t::metadata_change(const fs_meta_data &info)
{
	throw std::logic_error("metadata_change called on a filesystem not supporting write");
}

std::vector<uint8_t> filesystem_t::ifile_t::rsrc_read_all()
{
	throw std::logic_error("rsrc_read_all called on a filesystem without resource forks");
}
