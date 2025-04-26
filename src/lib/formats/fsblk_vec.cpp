// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<u8>

#include "fsblk_vec.h"

#include "strformat.h"

#include <algorithm>
#include <stdexcept>

namespace fs {

const u8 *fsblk_vec_t::blk_t::rodata() const
{
	return m_data;
}

u8 *fsblk_vec_t::blk_t::data()
{
	return m_data;
}

u32 fsblk_vec_t::block_count() const
{
	return m_data.size() / m_block_size;
}

fsblk_vec_t::block_t::ptr fsblk_vec_t::get(u32 id)
{
	if(id >= block_count())
		throw std::out_of_range(util::string_format("Block number overflow: requiring block %d on device of size %d (%d bytes, block size %d)", id, block_count(), m_data.size(), m_block_size));
	return std::make_shared<blk_t>(m_data.data() + m_block_size*id, m_block_size);
}

void fsblk_vec_t::fill_all(u8 data)
{
	std::fill(m_data.begin(), m_data.end(), data);
}

} // namespace fs
