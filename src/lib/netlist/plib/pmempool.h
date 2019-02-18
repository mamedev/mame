// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pmempool.h
 *
 */

#ifndef PMEMPOOL_H_
#define PMEMPOOL_H_

#include "palloc.h"
#include "pstream.h"
#include "pstring.h"
#include "ptypes.h"
#include "putil.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plib {

	template <typename P, typename T>
	struct pool_deleter
	{
		constexpr pool_deleter() noexcept = default;

		template<typename PU, typename U, typename = typename
		       std::enable_if<std::is_convertible< U*, T*>::value>::type>
	    pool_deleter(const pool_deleter<PU, U>&) noexcept { }

		void operator()(T *p) const
		{
			p->~T();
			P::free(p);
		}
	};

	//============================================================
	//  Memory pool
	//============================================================

	class mempool
	{
	private:
		struct block
		{
			block(mempool *mp)
			: m_num_alloc(0)
			, m_free(mp->m_min_alloc)
			, m_cur(0)
			, m_data(nullptr)
			, m_mempool(mp)
			{
				std::size_t alloc_bytes = (mp->m_min_alloc + mp->m_min_align - 1) & ~(mp->m_min_align - 1);
				m_data_allocated = static_cast<char *>(::operator new(alloc_bytes));
				void *r = m_data_allocated;
				std::align(mp->m_min_align, mp->m_min_alloc, r, alloc_bytes);
				m_data  = reinterpret_cast<char *>(r);
			}
			std::size_t m_num_alloc;
			std::size_t m_free;
			std::size_t m_cur;
			char *m_data;
			char *m_data_allocated;
			mempool *m_mempool;
		};

		struct info
		{
			info(block *b, std::size_t p) : m_block(b), m_pos(p) { }
			~info() = default;
			COPYASSIGNMOVE(info, default)

			block * m_block;
			std::size_t m_pos;
		};


		block * new_block()
		{
			auto *b = new block(this);
			m_blocks.push_back(b);
			return b;
		}


		static std::unordered_map<void *, info> &sinfo()
		{
			static std::unordered_map<void *, info> spinfo;
			return spinfo;
		}

		size_t m_min_alloc;
		size_t m_min_align;

		std::vector<block *> m_blocks;

	public:

		mempool(size_t min_alloc, size_t min_align)
		: m_min_alloc(min_alloc), m_min_align(min_align)
		{
		}

		COPYASSIGNMOVE(mempool, delete)

		~mempool()
		{

			for (auto & b : m_blocks)
			{
				if (b->m_num_alloc != 0)
				{
					plib::perrlogger("Found {} info blocks\n", sinfo().size());
					plib::perrlogger("Found block with {} dangling allocations\n", b->m_num_alloc);
				}
				::operator delete(b->m_data);
			}
		}

		void *alloc(size_t size, size_t align)
		{
			if (align < m_min_align)
				align = m_min_align;

			size_t rs = (size + align - 1) & ~(align - 1);
			for (auto &b : m_blocks)
			{
				if (b->m_free > rs)
				{
					b->m_free -= rs;
					b->m_num_alloc++;
					auto ret = reinterpret_cast<void *>(b->m_data + b->m_cur);
					auto capacity(rs);
					std::align(align, size, ret, capacity);
					sinfo().insert({ ret, info(b, b->m_cur)});
					b->m_cur += rs;

					return ret;
				}
			}
			{
				block *b = new_block();
				b->m_num_alloc = 1;
				b->m_free = m_min_alloc - rs;
				auto ret = reinterpret_cast<void *>(b->m_data + b->m_cur);
				auto capacity(rs);
				std::align(align, size, ret, capacity);
				sinfo().insert({ ret, info(b, b->m_cur)});
				b->m_cur += rs;
				return ret;
			}
		}

		static void free(void *ptr)
		{
			auto it = sinfo().find(ptr);
			if (it == sinfo().end())
				plib::terminate("mempool::free - pointer not found\n");
			info i = it->second;
			block *b = i.m_block;
			if (b->m_num_alloc == 0)
				plib::terminate("mempool::free - double free was called\n");
			else
			{
				//b->m_free = m_min_alloc;
				//b->cur_ptr = b->data;
			}
			b->m_num_alloc--;
			//printf("Freeing in block %p %lu\n", b, b->m_num_alloc);
			sinfo().erase(it);
		}

		template <typename T>
		using poolptr = plib::owned_ptr<T, pool_deleter<mempool, T>>;

		template<typename T, typename... Args>
		poolptr<T> make_poolptr(Args&&... args)
		{
			auto mem = this->alloc(sizeof(T), alignof(T));
			auto *obj = new (mem) T(std::forward<Args>(args)...);
			poolptr<T> a(obj, true);
			return std::move(a);
		}

	};

	class mempool_default
	{
	private:

		size_t m_min_alloc;
		size_t m_min_align;

	public:

		mempool_default(size_t min_alloc, size_t min_align)
		: m_min_alloc(min_alloc), m_min_align(min_align)
		{
		}

		COPYASSIGNMOVE(mempool_default, delete)

		~mempool_default() = default;

		void *alloc(size_t size)
		{
			plib::unused_var(m_min_alloc); // -Wunused-private-field fires without
			plib::unused_var(m_min_align);

			return ::operator new(size);
		}

		static void free(void *ptr)
		{
			::operator delete(ptr);
		}

		template <typename T>
		using poolptr = plib::owned_ptr<T, pool_deleter<mempool_default, T>>;

		template<typename T, typename... Args>
		poolptr<T> make_poolptr(Args&&... args)
		{
			auto mem(alloc(sizeof(T)));
			auto *obj = new (mem) T(std::forward<Args>(args)...);
			poolptr<T> a(obj, true);
			return std::move(a);
		}

	};


} // namespace plib

#endif /* PMEMPOOL_H_ */
