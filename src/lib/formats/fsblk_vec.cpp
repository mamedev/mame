// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<uint8_t>

#include "emu.h"
#include "fsblk_vec.h"

const uint8_t *fsblk_vec_t::blk_t::rodata()
{
	return m_data;
}

uint8_t *fsblk_vec_t::blk_t::data()
{
	return m_data;
}

void fsblk_vec_t::blk_t::drop_weak_references()
{
}

uint32_t fsblk_vec_t::block_count() const
{
	return m_data.size() / m_block_size;
}

fsblk_t::block_t fsblk_vec_t::get(uint32_t id)
{
	if(id >= block_count())
		fatalerror("Block number overflow: requiring block %d on device of size %d (%d bytes, block size %d)\n", id, block_count(), m_data.size(), m_block_size);
	return block_t(new blk_t(m_data.data() + m_block_size*id, m_block_size));
}

void fsblk_vec_t::fill(uint8_t data)
{
	std::fill(m_data.begin(), m_data.end(), data);
}

