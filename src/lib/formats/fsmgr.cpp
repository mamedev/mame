// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "emu.h"
#include "fsmgr.h"

void fsblk_t::iblock_t::ref()
{
	m_ref ++;
}

void fsblk_t::iblock_t::ref_weak()
{
	m_weak_ref ++;
}

void fsblk_t::iblock_t::unref()
{
	m_ref --;
	if(m_ref == 0) {
		if(m_weak_ref) {
			drop_weak_references();
			if(m_weak_ref)
				fatalerror("drop_weak_references kept %d active references\n", m_weak_ref);
		} else
			delete this;
	}
}

void fsblk_t::iblock_t::unref_weak()
{
	m_weak_ref --;
	if(m_weak_ref == 0 && m_ref == 0)
		delete this;
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

void filesystem_t::format(const fs_meta_data &meta)
{
	fatalerror("format called on a filesystem not supporting it.\n");
}

filesystem_t::dir_t filesystem_t::root()
{
	fatalerror("root called on a filesystem not supporting it.\n");
}

fs_meta_data filesystem_t::metadata()
{
	fatalerror("filesystem_t::metadata called on a filesystem not supporting it.\n");
}

void fsblk_t::set_block_size(uint32_t block_size)
{
	m_block_size = block_size;
}


uint8_t *fsblk_t::iblock_t::offset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		fatalerror("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size);
	return data() + off;
}

const uint8_t *fsblk_t::iblock_t::rooffset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		fatalerror("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size);
	return rodata() + off;
}

void fsblk_t::block_t::copy(u32 offset, const uint8_t *src, u32 size)
{
	uint8_t *blk = m_block->offset("copy", offset, size);
	memcpy(blk, src, size);
}

void fsblk_t::block_t::fill(u32 offset, uint8_t data, u32 size)
{
	uint8_t *blk = m_block->offset("fill", offset, size);
	memset(blk, data, size);
}

void fsblk_t::block_t::fill(uint8_t data)
{
	uint8_t *blk = m_block->data();
	memset(blk, data, m_block->size());
}

void fsblk_t::block_t::wstr(u32 offset, const std::string &str)
{
	uint8_t *blk = m_block->offset("wstr", offset, str.size());
	memcpy(blk, str.data(), str.size());
}

void fsblk_t::block_t::w8(u32 offset, uint8_t data)
{
	uint8_t *blk = m_block->offset("w8", offset, 1);
	blk[0] = data;
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	uint8_t *blk = m_block->offset("w16b", offset, 2);
	blk[0] = data >> 8;
	blk[1] = data;
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	uint8_t *blk = m_block->offset("w32b", offset, 4);
	blk[0] = data >> 24;
	blk[1] = data >> 16;
	blk[2] = data >> 8;
	blk[3] = data;
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	uint8_t *blk = m_block->offset("w16l", offset, 2);
	blk[0] = data;
	blk[1] = data >> 8;
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	uint8_t *blk = m_block->offset("w32l", offset, 4);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
	blk[3] = data >> 24;
}

std::string fsblk_t::block_t::rstr(u32 offset, u32 size)
{
	const u8 *d = m_block->rooffset("rstr", offset, size);
	std::string res;
	for(u32 i=0; i != size; i++)
		res += char(*d++);
	return res;
}

const char *fs_meta_get_name(fs_meta_name name)
{
	switch(name) {
	case fs_meta_name::creation_date: return "creation_date";
	case fs_meta_name::length: return "length";
	case fs_meta_name::loading_address: return "loading_address";
	case fs_meta_name::locked: return "locked";
	case fs_meta_name::sequential: return "sequential";
	case fs_meta_name::modification_date: return "modification_date";
	case fs_meta_name::name: return "name";
	case fs_meta_name::size_in_blocks: return "size_in_blocks";
	}
	return "";
}

std::string fs_meta_to_string(fs_meta_type type, const fs_meta &m)
{
	switch(type) {
	case fs_meta_type::string: return std::get<std::string>(m);
	case fs_meta_type::number: return util::string_format("0x%x", std::get<uint64_t>(m));
	case fs_meta_type::flag:   return std::get<bool>(m) ? "t" : "f";
	case fs_meta_type::date:   abort();
	}
	return std::string("");
}

