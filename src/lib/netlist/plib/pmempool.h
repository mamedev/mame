// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PMEMPOOL_H_
#define PMEMPOOL_H_

///
/// \file pmempool.h
///

#include "palloc.h"
#include "pconfig.h"
#include "pstream.h"
#include "pstring.h"
#include "ptypes.h"
#include "putil.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plib {

	//============================================================
	//  Memory pool
	//============================================================

	template <typename BASEARENA>
	class mempool_arena : public arena_base<mempool_arena<BASEARENA>, false, false>
	{
	public:

		using size_type = typename BASEARENA::size_type;
		using base_type = arena_base<mempool_arena<BASEARENA>, false, false>;
		template <class T>
		using base_allocator_type = typename BASEARENA::template allocator_type<T>;

		mempool_arena(size_t min_alloc = (1<<21), size_t min_align = PALIGN_CACHELINE)
		: m_min_alloc(min_alloc)
		, m_min_align(min_align)
		, m_blocks(base_allocator_type<block *>(m_arena))
		{
			icount()++;
		}

		PCOPYASSIGNMOVE(mempool_arena, delete)

		~mempool_arena()
		{

			for (auto & b : m_blocks)
			{
				if (b->m_num_alloc != 0)
				{
					plib::perrlogger("Found {} info blocks\n", sinfo().size());
					plib::perrlogger("Found block with {} dangling allocations\n", b->m_num_alloc);
				}
				m_arena.free(b);
				//::operator delete(b->m_data);
			}
			if (icount()-- == 1)
			{
				if (!sinfo().empty())
					plib::perrlogger("Still found {} info blocks after last mempool deleted\n", sinfo().size());
			}
		}

		void *allocate(size_t align, size_t size)
		{
			block *b = nullptr;

			if (align < m_min_align)
				align = m_min_align;

			size_t rs = size + align;
			for (auto &bs : m_blocks)
			{
				if (bs->m_free > rs)
				{
					b = bs;
					break;
				}
			}
			if (b == nullptr)
			{
				b = new_block(rs);
			}
			b->m_free -= rs;
			b->m_num_alloc++;
			void *ret = reinterpret_cast<void *>(b->m_data + b->m_cur);
			auto capacity(rs);
			ret = std::align(align, size, ret, capacity);
			sinfo().insert({ ret, info(b, b->m_cur)});
			rs -= (capacity - size);
			b->m_cur += rs;
			base_type::m_stat_cur_alloc() += size;
			if (base_type::m_stat_max_alloc() < base_type::m_stat_cur_alloc())
				base_type::m_stat_max_alloc() = base_type::m_stat_cur_alloc();

			return ret;
		}

		/*static */ void deallocate(void *ptr, size_t size) noexcept
		{

			auto it = sinfo().find(ptr);
			if (it == sinfo().end())
				plib::terminate("mempool::free - pointer not found");
			block *b = it->second.m_block;
			if (b->m_num_alloc == 0)
				plib::terminate("mempool::free - double free was called");
			else
			{
				mempool_arena &mp = b->m_mempool;
				b->m_num_alloc--;
				mp.m_stat_cur_alloc() -= size;
				if (b->m_num_alloc == 0)
				{
					auto itb = std::find(mp.m_blocks.begin(), mp.m_blocks.end(), b);
					if (itb == mp.m_blocks.end())
						plib::terminate("mempool::free - block not found");

					mp.m_blocks.erase(itb);
					m_arena.free(b);
				}
				sinfo().erase(it);
			}
		}

		bool operator ==(const mempool_arena &rhs) const noexcept { return this == &rhs; }

	private:
		struct block
		{
			block(mempool_arena &mp, size_type min_bytes)
			: m_num_alloc(0)
			, m_cur(0)
			, m_data(nullptr)
			, m_mempool(mp)
			{
				min_bytes = std::max(mp.m_min_alloc, min_bytes);
				m_free = min_bytes;
				size_type alloc_bytes = (min_bytes + mp.m_min_align); // - 1); // & ~(mp.m_min_align - 1);
				// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
				m_data_allocated = new std::uint8_t[alloc_bytes];
				void *r = m_data_allocated;
				std::align(mp.m_min_align, min_bytes, r, alloc_bytes);
				m_data  = reinterpret_cast<std::uint8_t *>(r);
			}
			~block()
			{
				//::operator delete(m_data_allocated);
				delete [] m_data_allocated;
			}

			block(const block &) = delete;
			block(block &&) = delete;
			block &operator =(const block &) = delete;
			block &operator =(block &&) = delete;

			size_type m_num_alloc;
			size_type m_free;
			size_type m_cur;
			std::uint8_t *m_data;
			std::uint8_t *m_data_allocated;
			mempool_arena &m_mempool;
		};

		struct info
		{
			info(block *b, size_type p) : m_block(b), m_pos(p) { }
			~info() = default;
			PCOPYASSIGNMOVE(info, default)

			block * m_block;
			size_type m_pos;
		};

		block * new_block(size_type min_bytes)
		{
			auto *b = m_arena.template alloc<block>(*this, min_bytes);
			m_blocks.push_back(b);
			return b;
		}

		static std::unordered_map<void *, info> &sinfo()
		{
			static std::unordered_map<void *, info> spinfo;
			return spinfo;
		}

		static std::size_t &icount()
		{
			static std::size_t count = 0;
			return count;
		}

		size_t m_min_alloc;
		size_t m_min_align;
		BASEARENA m_arena;

		std::vector<block *, typename BASEARENA::template allocator_type<block *>> m_blocks;

	};

} // namespace plib

#endif // PMEMPOOL_H_
