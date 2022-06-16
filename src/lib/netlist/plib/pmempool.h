// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PMEMPOOL_H_
#define PMEMPOOL_H_

///
/// \file pmempool.h
///

#include "palloc.h"
#include "pconfig.h"
#include "pstream.h"  // perrlogger
//#include "pstring.h"
#include "ptypes.h"
//#include "putil.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plib {

	//============================================================
	//  Memory pool
	//============================================================

	template <typename BASEARENA, std::size_t MINALIGN = PALIGN_MIN_SIZE>
	class mempool_arena : public arena_base<mempool_arena<BASEARENA, MINALIGN>, MINALIGN, false, false>
	{
	public:

		using size_type = typename BASEARENA::size_type;
		using base_type = arena_base<mempool_arena<BASEARENA, MINALIGN>, MINALIGN, false, false>;

		template <class T>
		using base_allocator_type = typename BASEARENA::template allocator_type<T>;
		using block_align = std::integral_constant<size_t, 1024>;

		mempool_arena(size_t min_alloc = (1<<21));

		PCOPYASSIGNMOVE(mempool_arena, delete)

		~mempool_arena();

		void *allocate(size_t align, size_t size);

		void deallocate(void *ptr, std::size_t alignment, size_t size) noexcept;

		bool operator ==(const mempool_arena &rhs) const noexcept { return this == &rhs; }

	private:
		struct block
		{
			block(mempool_arena &mp, size_type min_bytes);
			~block()
			{
				//::operator delete(m_data_allocated);
				//delete [] m_data_allocated;
				m_mempool.base_arena().deallocate(m_data_allocated, mempool_arena::block_align(), m_bytes_allocated);
			}

			block(const block &) = delete;
			block(block &&) = delete;
			block &operator =(const block &) = delete;
			block &operator =(block &&) = delete;

			size_type m_num_alloc;
			size_type m_free;
			size_type m_cur;
			size_type m_bytes_allocated;
			std::uint8_t *m_data;
			std::uint8_t *m_data_allocated;
			mempool_arena &m_mempool;
		};

		struct info
		{
			info(block *b, size_type p, size_type orig_size) : m_block(b), m_pos(p), m_orig_size(orig_size) { }
			info(const info &) = default;
			info &operator=(const info &) = default;
			info(info &&) noexcept = default;
			info &operator=(info &&) noexcept = default;
			~info() = default;

			block * m_block;
			size_type m_pos;
			size_type m_orig_size;
		};

		block * new_block(size_type min_bytes)
		{
			auto *b = detail::alloc<block, 0>(base_arena(), *this, min_bytes);
			m_blocks.push_back(b);
			return b;
		}

		size_t m_min_alloc;
		static BASEARENA &base_arena()
		{
			static BASEARENA s_arena;
			return s_arena;
		}

		using info_type = std::pair<void * const, info>;

		using info_allocator_type = typename BASEARENA::template allocator_type<info_type>;
		std::unordered_map<void *, info, std::hash<void *>, std::equal_to<>,
			info_allocator_type> m_info;

		plib::arena_vector<BASEARENA, block *> m_blocks;

	};

	template <typename BASEARENA, std::size_t MINALIGN>
	mempool_arena<BASEARENA, MINALIGN>::mempool_arena(size_t min_alloc)
	: m_min_alloc(min_alloc)
	//, m_blocks(base_allocator_type<block *>(base_arena()))
	, m_blocks(base_arena())
	{
	}

	template <typename BASEARENA, std::size_t MINALIGN>
	mempool_arena<BASEARENA, MINALIGN>::~mempool_arena()
	{
		for (auto & b : m_blocks)
		{
			if (b->m_num_alloc != 0)
			{
				plib::perrlogger("Found {} info blocks\n", m_info.size());
				plib::perrlogger("Found block with {} dangling allocations\n", b->m_num_alloc);
			}
			detail::free<0>(base_arena(), b);
		}
		if (!m_info.empty())
			plib::perrlogger("Still found {} info blocks after mempool deleted\n", m_info.size());
	}

	template <typename BASEARENA, std::size_t MINALIGN>
	void * mempool_arena<BASEARENA, MINALIGN>::allocate(size_t align, size_t size)
	{
		block *b = nullptr;

		if (align < MINALIGN)
			align = MINALIGN;

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
		void *ret = reinterpret_cast<void *>(b->m_data + b->m_cur); // NOLINT(cppcoreguidelines-pro-type-reinterpret
		auto capacity(rs);
		ret = std::align(align, size, ret, capacity);
		m_info.insert({ ret, info(b, b->m_cur, size)});
		rs -= (capacity - size);
		b->m_cur += rs;
		this->inc_alloc_stat(size);

		return ret;
	}

	template <typename BASEARENA, std::size_t MINALIGN>
	void mempool_arena<BASEARENA, MINALIGN>::deallocate(void *ptr, [[maybe_unused]] std::size_t alignment, size_t size) noexcept
	{
		auto it = m_info.find(ptr);
		if (it == m_info.end())
			plib::terminate("mempool::free - pointer not found");
		block *b = it->second.m_block;
		if (b->m_num_alloc == 0)
			plib::terminate("mempool::free - double free was called");
		if (it->second.m_orig_size != size)
			plib::perrlogger("Size mismatch original {} vs {}\n", it->second.m_orig_size, size);

		mempool_arena &mp = b->m_mempool;
		b->m_num_alloc--;
		mp.dec_alloc_stat(size);
		if (b->m_num_alloc == 0)
		{
			auto itb = std::find(mp.m_blocks.begin(), mp.m_blocks.end(), b);
			if (itb == mp.m_blocks.end())
				plib::terminate("mempool::free - block not found");

			mp.m_blocks.erase(itb);
			detail::free<0>(mp.base_arena(), b);
		}
		m_info.erase(it);
	}

	template <typename BASEARENA, std::size_t MINALIGN>
	mempool_arena<BASEARENA, MINALIGN>::block::block(mempool_arena &mp, size_type min_bytes)
	: m_num_alloc(0)
	, m_cur(0)
	, m_data(nullptr)
	, m_mempool(mp)
	{
		min_bytes = std::max(mp.m_min_alloc, min_bytes);
		m_free = min_bytes;
		m_bytes_allocated = (min_bytes + mempool_arena::block_align()); // - 1); // & ~(mp.m_min_align - 1);
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		//m_data_allocated = new std::uint8_t[alloc_bytes];
		m_data_allocated = static_cast<std::uint8_t *>(mp.base_arena().allocate(mempool_arena::block_align(), m_bytes_allocated));
		void *r = m_data_allocated;
		std::align(mempool_arena::block_align(), min_bytes, r, m_bytes_allocated);
		m_data  = reinterpret_cast<std::uint8_t *>(r); // NOLINT(cppcoreguidelines-pro-type-reinterpret
	}

} // namespace plib

#endif // PMEMPOOL_H_
