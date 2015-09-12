/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "test.h"
#include <bx/handlealloc.h>

TEST(HandleListT)
{
	bx::HandleListT<32> list;

	list.pushBack(16);
	CHECK(list.getFront() == 16);
	CHECK(list.getBack()  == 16);

	list.pushFront(7);
	CHECK(list.getFront() ==  7);
	CHECK(list.getBack()  == 16);

	uint16_t expected0[] = { 15, 31, 7, 16, 17, 11, 13 };
	list.pushBack(17);
	list.pushBack(11);
	list.pushBack(13);
	list.pushFront(31);
	list.pushFront(15);
	uint16_t count = 0;
	for (uint16_t it = list.getFront(); it != UINT16_MAX; it = list.getNext(it), ++count)
	{
		CHECK(it == expected0[count]);
	}
	CHECK(count == BX_COUNTOF(expected0) );

	list.remove(17);
	list.remove(31);
	list.remove(16);
	list.pushBack(16);
	uint16_t expected1[] = { 15, 7, 11, 13, 16 };
	count = 0;
	for (uint16_t it = list.getFront(); it != UINT16_MAX; it = list.getNext(it), ++count)
	{
		CHECK(it == expected1[count]);
	}
	CHECK(count == BX_COUNTOF(expected1) );

	list.popBack();
	list.popFront();
	list.popBack();
	list.popBack();

	CHECK(list.getFront() ==  7);
	CHECK(list.getBack()  ==  7);

	list.popBack();
	CHECK(list.getFront() ==  UINT16_MAX);
	CHECK(list.getBack()  ==  UINT16_MAX);
}

TEST(HandleAllocLruT)
{
	bx::HandleAllocLruT<16> lru;

	uint16_t handle[4] =
	{
		lru.alloc(),
		lru.alloc(),
		lru.alloc(),
		lru.alloc(),
	};

	lru.touch(handle[1]);

	uint16_t expected0[] = { handle[1], handle[3], handle[2], handle[0] };
	uint16_t count = 0;
	for (uint16_t it = lru.getFront(); it != UINT16_MAX; it = lru.getNext(it), ++count)
	{
		CHECK(it == expected0[count]);
	}
}
