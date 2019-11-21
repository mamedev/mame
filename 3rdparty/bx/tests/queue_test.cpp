/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/spscqueue.h>
#include <bx/mpscqueue.h>

void* bitsToPtr(uintptr_t _ui)
{
	union { uintptr_t ui; void* ptr; } cast = { _ui };
	return cast.ptr;
}

uintptr_t ptrToBits(void* _ptr)
{
	union { void* ptr; uintptr_t ui; } cast = { _ptr };
	return cast.ui;
}

TEST_CASE("SpSc", "")
{
	bx::DefaultAllocator allocator;
	bx::SpScUnboundedQueue queue(&allocator);
	queue.push(bitsToPtr(0xdeadbeef) );
	REQUIRE(0xdeadbeef == ptrToBits(queue.pop() ) );
}

TEST_CASE("MpSc", "")
{
	bx::DefaultAllocator allocator;
	bx::MpScUnboundedQueueT<void> queue(&allocator);
	queue.push(bitsToPtr(0xdeadbeef) );
	REQUIRE(0xdeadbeef == ptrToBits(queue.pop() ) );
}
