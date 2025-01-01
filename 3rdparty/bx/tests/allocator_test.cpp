/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"

#include <bx/allocator.h>

struct MockNonFreeingAllocator : public bx::AllocatorI
{
	MockNonFreeingAllocator()
		: m_sbrk(1)
	{
	}

	~MockNonFreeingAllocator() override
	{
	}

	void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) override
	{
		BX_ASSERT(_ptr == NULL, "MockNonFreeingAllocator can't realloc or free.");
		BX_ASSERT(m_sbrk + _size < BX_COUNTOF(m_storage), "");
		BX_UNUSED(_ptr, _file, _line);

		const uintptr_t sbrk = bx::alignUp(m_sbrk, bx::max(int32_t(_align), 1) );
		m_sbrk = sbrk + _size;

		return &m_storage[sbrk];
	}

	uintptr_t m_sbrk;
	BX_ALIGN_DECL(256, uint8_t) m_storage[0x10000];
};

bool testAlignment(size_t _expected, void* _ptr)
{
	bool aligned = bx::isAligned(_ptr, int32_t(_expected) );
//	BX_TRACE("%p, %d", _ptr, _expected);
	BX_WARN(aligned, "%p not aligned to %d bytes.", _ptr, _expected);
	return aligned;
}

TEST_CASE("Allocator", "")
{
	MockNonFreeingAllocator mnfa;

	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(2,   bx::alloc       (&mnfa, 1, 2  ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(4,   bx::alloc       (&mnfa, 1, 4  ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(8,   bx::alloc       (&mnfa, 1, 8  ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(16,  bx::alloc       (&mnfa, 1, 16 ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(32,  bx::alloc       (&mnfa, 1, 32 ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(64,  bx::alloc       (&mnfa, 1, 64 ) ) );
	REQUIRE(testAlignment(1,   bx::alloc       (&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(128, bx::alloc       (&mnfa, 1, 128) ) );

	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(2,   bx::alignedAlloc(&mnfa, 1, 2  ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(4,   bx::alignedAlloc(&mnfa, 1, 4  ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(8,   bx::alignedAlloc(&mnfa, 1, 8  ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(16,  bx::alignedAlloc(&mnfa, 1, 16 ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(32,  bx::alignedAlloc(&mnfa, 1, 32 ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(64,  bx::alignedAlloc(&mnfa, 1, 64 ) ) );
	REQUIRE(testAlignment(1,   bx::alignedAlloc(&mnfa, 1, 1  ) ) );
	REQUIRE(testAlignment(128, bx::alignedAlloc(&mnfa, 1, 128) ) );
}
