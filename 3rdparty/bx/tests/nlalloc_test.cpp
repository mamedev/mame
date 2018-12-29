/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/sort.h>
#include <bx/allocator.h>

bx::DefaultAllocator g_allocator0;

struct TinyStlAllocator
{
	static void* static_allocate(size_t _bytes)
	{
		return BX_ALLOC(&g_allocator0, _bytes);
	}

	static void static_deallocate(void* _ptr, size_t /*_bytes*/)
	{
		if (NULL != _ptr)
		{
			BX_FREE(&g_allocator0, _ptr);
		}
	}
};

#define TINYSTL_ALLOCATOR ::TinyStlAllocator
#include <tinystl/vector.h>

namespace stl = tinystl;

namespace bx
{
	struct Blk
	{
		static const uint64_t kInvalid = UINT64_MAX;

		Blk()
			: ptr(kInvalid)
			, size(0)
		{
		}

		Blk(uint64_t _ptr, uint32_t _size)
			: ptr(_ptr)
			, size(_size)
		{
		}

		uint64_t ptr;
		uint32_t size;
	};

	inline bool operator<(const Blk& _lhs, const Blk& _rhs)
	{
		return _lhs.ptr < _rhs.ptr;
	}

	inline bool isValid(const Blk& _blk)
	{
		return Blk::kInvalid != _blk.ptr;
	}

	// First-fit non-local allocator.
	class NonLocalAllocator
	{
	public:
		NonLocalAllocator()
		{
			reset();
		}

		~NonLocalAllocator()
		{
		}

		void reset()
		{
			m_free.clear();
			m_used = 0;
		}

		void add(const Blk& _blk)
		{
			m_free.push_back(_blk);
		}

		Blk remove()
		{
			BX_CHECK(0 == m_used, "");

			if (0 < m_free.size() )
			{
				Blk freeBlock = m_free.back();
				m_free.pop_back();
				return freeBlock;
			}

			return Blk{};
		}

		Blk alloc(uint32_t _size)
		{
			_size = max(_size, 16u);

			for (FreeList::iterator it = m_free.begin(), itEnd = m_free.end(); it != itEnd; ++it)
			{
				if (it->size >= _size)
				{
					uint64_t ptr = it->ptr;

					if (it->size != _size)
					{
						it->size -= _size;
						it->ptr  += _size;
					}
					else
					{
						m_free.erase(it);
					}

					m_used += _size;
					return Blk{ ptr, _size };
				}
			}

			// there is no block large enough.
			return Blk{};
		}

		void free(const Blk& _blk)
		{
			m_used -= _blk.size;
			m_free.push_back(_blk);
			compact();
		}

		bool compact()
		{
			bx::quickSort(
				  m_free.begin()
				, uint32_t(m_free.end() - m_free.begin() )
				, sizeof(Blk)
				, [](const void* _a, const void* _b) -> int32_t {
					const Blk& lhs = *(const Blk*)(_a);
					const Blk& rhs = *(const Blk*)(_b);
					return lhs < rhs ? -1 : 1;
				});

			for (FreeList::iterator it = m_free.begin(), next = it, itEnd = m_free.end(); next != itEnd;)
			{
				if ( (it->ptr + it->size) == next->ptr)
				{
					it->size += next->size;
					next = m_free.erase(next);
				}
				else
				{
					it = next;
					++next;
				}
			}

			return 0 == m_used;
		}

		uint32_t getUsed() const
		{
			return m_used;
		}

	private:
		typedef stl::vector<Blk> FreeList;
		FreeList m_free;
		uint32_t m_used;
	};
} // namespace bx

TEST_CASE("nlalloc")
{
	bx::NonLocalAllocator nla;

	bx::Blk blk;

	blk = nla.alloc(100);
	REQUIRE(!isValid(blk) );
	nla.add(bx::Blk{0x1000, 100});

	blk = nla.alloc(100);
	REQUIRE(isValid(blk) );

	nla.free(blk);
	REQUIRE(0 == nla.getUsed() );
}
