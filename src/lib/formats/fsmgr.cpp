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


void filesystem_manager_t::enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
}

void filesystem_manager_t::enumerate(hd_enumerator &he) const
{
}

void filesystem_manager_t::enumerate(cdrom_enumerator &ce) const
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

void filesystem_t::format()
{
	fatalerror("format called on filesystem not supporting it.\n");
}

filesystem_t::dir_t filesystem_t::root()
{
	fatalerror("root called on filesystem not supporting it.\n");
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
