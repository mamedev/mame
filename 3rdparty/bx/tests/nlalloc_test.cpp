/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
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
		static constexpr uint64_t kInvalid = UINT64_MAX;

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
		static constexpr uint32_t kMinBlockSize = 16u;

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
			BX_ASSERT(0 == m_used, "");

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
			_size = max(_size, kMinBlockSize);

			for (FreeList::iterator it = m_free.begin(), itEnd = m_free.end(); it != itEnd; ++it)
			{
				if (it->size >= _size)
				{
					const uint64_t ptr = it->ptr;

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
			BX_ASSERT(isValid(_blk), "Freeing invalid block!");

			m_used -= _blk.size;

			FreeList::iterator it    = m_free.begin();
			FreeList::iterator itEnd = m_free.end();
			for (; it != itEnd; ++it)
			{
				if ( (_blk.ptr + _blk.size) == it->ptr)
				{
					it->ptr   = _blk.ptr;
					it->size += _blk.size;
					break;
				}

				if (_blk.ptr > it->ptr)
				{
					m_free.insert(it, _blk);
					break;
				}
			}

			if (it == itEnd)
			{
				m_free.push_back(_blk);
			}
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

	constexpr uint32_t NonLocalAllocator::kMinBlockSize;

} // namespace bx

TEST_CASE("nlalloc")
{
	bx::NonLocalAllocator nla;

	bx::Blk blk;

	blk = nla.alloc(100);
	REQUIRE(!isValid(blk) );
	nla.add(bx::Blk{0x1000, 1024});

	blk = nla.alloc(1024);
	REQUIRE(isValid(blk) );

	bx::Blk blk2 = nla.alloc(1);
	REQUIRE(!isValid(blk2) );

	nla.free(blk);
	REQUIRE(0 == nla.getUsed() );

	{
		bx::Blk blk3 = nla.alloc(123);
		REQUIRE(isValid(blk3) );

		bx::Blk blk4 = nla.alloc(134);
		REQUIRE(isValid(blk4) );

		bx::Blk blk5 = nla.alloc(145);
		REQUIRE(isValid(blk5) );

		bx::Blk blk6 = nla.alloc(156);
		REQUIRE(isValid(blk6) );

		bx::Blk blk7 = nla.alloc(167);
		REQUIRE(isValid(blk7) );

		nla.free(blk3);
		nla.free(blk5);
		nla.free(blk7);
		nla.free(blk4);
		nla.free(blk6);
		REQUIRE(0 == nla.getUsed() );
	}

	blk2 = nla.alloc(1);
	REQUIRE(isValid(blk2) );
	REQUIRE(bx::NonLocalAllocator::kMinBlockSize == nla.getUsed() );

	nla.free(blk2);
	REQUIRE(0 == nla.getUsed() );
}
