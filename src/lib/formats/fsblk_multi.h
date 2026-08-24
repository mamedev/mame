// license:BSD-3-Clause
// copyright-holders:AJR

// Address several instances of fsblk_t::block_t as a single concatenated block

#include "fsblk.h"

#include <array>
#include <utility>

namespace fs {

template <int N>
class multi_block final : public fsblk_t::block_t
{
public:
	template <typename... Params> multi_block(Params &&... args) requires (sizeof...(args) == N)
		: block_t(([&args...] () { u32 size = 0; for (const fsblk_t::block_t::ptr &block : { args... }) { size += block->size(); } return size; })())
		, m_blocks{ std::move(args)... }
	{
	}

protected:
	virtual void internal_write(u32 offset, const u8 *src, u32 size) override
	{
		for (int i = 0; i < N; i++)
		{
			// no need to check size of last block
			if (i == N - 1)
			{
				m_blocks[i]->write(offset, src, size);
				return;
			}

			const u32 blksize = m_blocks[i]->size();
			if (offset < blksize)
			{
				const u32 lowpart = blksize - offset;
				if (size <= lowpart)
				{
					m_blocks[i]->write(offset, src, size);
					return;
				}
				else
				{
					m_blocks[i]->write(offset, src, lowpart);
					offset = 0;
					src += lowpart;
					size -= lowpart;
				}
			}
			else
				offset -= blksize;
		}
	}

	virtual void internal_fill(u32 offset, u8 data, u32 size) override
	{
		for (int i = 0; i < N; i++)
		{
			// no need to check size of last block
			if (i == N - 1)
			{
				m_blocks[i]->fill(offset, data, size);
				return;
			}

			const u32 blksize = m_blocks[i]->size();
			if (offset < blksize)
			{
				const u32 lowpart = blksize - offset;
				if (size <= lowpart)
				{
					m_blocks[i]->fill(offset, data, size);
					return;
				}
				else
				{
					m_blocks[i]->fill(offset, data, lowpart);
					offset = 0;
					size -= lowpart;
				}
			}
			else
				offset -= blksize;
		}
	}

	virtual void internal_read(u32 offset, u8 *dst, u32 size) const override
	{
		for (int i = 0; i < N; i++)
		{
			// no need to check size of last block
			if (i == N - 1)
			{
				m_blocks[i]->read(offset, dst, size);
				return;
			}

			const u32 blksize = m_blocks[i]->size();
			if (offset < blksize)
			{
				const u32 lowpart = blksize - offset;
				if (size <= lowpart)
				{
					m_blocks[i]->read(offset, dst, size);
					return;
				}
				else
				{
					m_blocks[i]->read(offset, dst, lowpart);
					offset = 0;
					dst += lowpart;
					size -= lowpart;
				}
			}
			else
				offset -= blksize;
		}
	}

	virtual bool internal_eqmem(u32 offset, const u8 *src, u32 size) const override
	{
		for (int i = 0; i < N; i++)
		{
			// no need to check size of last block
			if (i == N - 1)
				return m_blocks[i]->eqmem(offset, src, size);

			const u32 blksize = m_blocks[i]->size();
			if (offset < blksize)
			{
				const u32 lowpart = blksize - offset;
				if (size <= lowpart)
					return m_blocks[i]->eqmem(offset, src, size);
				else
				{
					if (!m_blocks[i]->eqmem(offset, src, lowpart))
						return false;
					offset = 0;
					src += lowpart;
					size -= lowpart;
				}
			}
			else
				offset -= blksize;
		}

		// normally never reached
		return true;
	}

private:
	std::array<fsblk_t::block_t::ptr, N> m_blocks;
};

} // namespace fs
