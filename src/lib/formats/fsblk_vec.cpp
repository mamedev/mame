// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<u8>

#include "fsblk_vec.h"

#include "strformat.h"

#include <algorithm>
#include <stdexcept>

namespace fs {

const u8 *fsblk_vec_base_t::blk_t::rodata()
{
	return m_data;
}

u8 *fsblk_vec_base_t::blk_t::data()
{
	return m_data;
}

void fsblk_vec_base_t::blk_t::drop_weak_references()
{
}

u32 fsblk_vec_base_t::block_count() const
{
	return vec().size() / m_block_size;
}

fsblk_t::block_t fsblk_vec_base_t::get(u32 id)
{
	if(id >= block_count())
		throw std::out_of_range(util::string_format("Block number overflow: requiring block %d on device of size %d (%d bytes, block size %d)\n", id, block_count(), vec().size(), m_block_size));
	return block_t(new blk_t(vec().data() + m_block_size*id, m_block_size));
}

void fsblk_vec_base_t::fill(u8 data)
{
	std::fill(vec().begin(), vec().end(), data);
}

} // namespace fs
