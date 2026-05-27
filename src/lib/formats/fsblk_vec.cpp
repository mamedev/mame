// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<u8>

#include "fsblk_vec.h"

#include "strformat.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace fs {

void fsblk_vec_t::blk_t::internal_write(u32 offset, const u8 *src, u32 size)
{
	std::copy_n(src, size, m_data + offset);
}

void fsblk_vec_t::blk_t::internal_fill(u32 offset, u8 data, u32 size)
{
	std::fill_n(m_data + offset, size, data);
}

void fsblk_vec_t::blk_t::internal_read(u32 offset, u8 *dst, u32 size) const
{
	std::copy_n(m_data + offset, size, dst);
}

bool fsblk_vec_t::blk_t::internal_eqmem(u32 offset, const u8 *src, u32 size) const
{
	return std::equal(src, src + size, m_data + offset);
}

u32 fsblk_vec_t::block_count() const
{
	assert(m_block_size);
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
