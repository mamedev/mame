/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/handlealloc.h>
#include <bx/hash.h>
#include <bx/rng.h>

#include <set>

TEST_CASE("HandleAllocT", "")
{
	constexpr int Max = 32;
	bx::HandleAllocT<Max> alloc{};

	REQUIRE(sizeof(alloc) == sizeof(uint16_t) * Max * 2 + sizeof(bx::HandleAlloc));

	for (uint16_t i = 0; i < Max; ++i)
	{
		REQUIRE(!alloc.isValid(i));
	}

	std::set<uint16_t> reference_handles;
	int count = 0;
	bx::RngMwc random;
	for (int i = 0; i < 200000; ++i)
	{
		bool add = random.gen() % 2;
		if (add && count < Max)
		{
			count++;
			uint16_t new_handle = alloc.alloc();
			reference_handles.insert(new_handle);
		}
		else if (count > 0)
		{
			count--;
			int idx = rand() % reference_handles.size();
			auto it = reference_handles.begin();
			for (int it_idx = 0; it_idx < idx; ++it_idx) {
				it++;
				REQUIRE(alloc.isValid(*it));
			}
			uint16_t handle_to_remove = *it;
			alloc.free(handle_to_remove);
			REQUIRE(!alloc.isValid(handle_to_remove));
			reference_handles.erase(it);
		}

		// Check if it's still correct
		for (auto it = reference_handles.begin(); it != reference_handles.end(); ++it)
		{
			REQUIRE(alloc.isValid(*it));
		}
	}

	// Finally delete all
	for (auto it = reference_handles.begin(); it != reference_handles.end(); ++it)
	{
		REQUIRE(alloc.isValid(*it));
		alloc.free(*it);
	}
	reference_handles.clear();

	for (uint16_t i = 0; i < Max; ++i) {
		REQUIRE(!alloc.isValid(i));
	}

}

TEST_CASE("HandleListT", "")
{
	bx::HandleListT<32> list;

	list.pushBack(16);
	REQUIRE(list.getFront() == 16);
	REQUIRE(list.getBack()  == 16);

	list.pushFront(7);
	REQUIRE(list.getFront() ==  7);
	REQUIRE(list.getBack()  == 16);

	uint16_t expected0[] = { 15, 31, 7, 16, 17, 11, 13 };
	list.pushBack(17);
	list.pushBack(11);
	list.pushBack(13);
	list.pushFront(31);
	list.pushFront(15);
	uint16_t count = 0;
	for (uint16_t it = list.getFront(); it != UINT16_MAX; it = list.getNext(it), ++count)
	{
		REQUIRE(it == expected0[count]);
	}
	REQUIRE(count == BX_COUNTOF(expected0) );

	list.remove(17);
	list.remove(31);
	list.remove(16);
	list.pushBack(16);
	uint16_t expected1[] = { 15, 7, 11, 13, 16 };
	count = 0;
	for (uint16_t it = list.getFront(); it != UINT16_MAX; it = list.getNext(it), ++count)
	{
		REQUIRE(it == expected1[count]);
	}
	REQUIRE(count == BX_COUNTOF(expected1) );

	list.popBack();
	list.popFront();
	list.popBack();
	list.popBack();

	REQUIRE(list.getFront() ==  7);
	REQUIRE(list.getBack()  ==  7);

	list.popBack();
	REQUIRE(list.getFront() ==  UINT16_MAX);
	REQUIRE(list.getBack()  ==  UINT16_MAX);
}

TEST_CASE("HandleAllocLruT", "")
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
		REQUIRE(it == expected0[count]);
	}
}

TEST_CASE("HandleHashTable", "")
{
	typedef bx::HandleHashMapT<512> HashMap;

	HashMap hm;

	REQUIRE(512 == hm.getMaxCapacity() );

	bx::StringView sv0("test0");

	bool ok = hm.insert(bx::hash<bx::HashMurmur2A>(sv0), 0);
	REQUIRE(ok);

	ok = hm.insert(bx::hash<bx::HashMurmur2A>(sv0), 0);
	REQUIRE(!ok);
	REQUIRE(1 == hm.getNumElements() );

	bx::StringView sv1("test1");

	ok = hm.insert(bx::hash<bx::HashMurmur2A>(sv1), 0);
	REQUIRE(ok);
	REQUIRE(2 == hm.getNumElements() );

	hm.removeByHandle(0);
	REQUIRE(0 == hm.getNumElements() );

	ok = hm.insert(bx::hash<bx::HashMurmur2A>(sv0), 0);
	REQUIRE(ok);

	hm.removeByKey(bx::hash<bx::HashMurmur2A>(sv0) );
	REQUIRE(0 == hm.getNumElements() );

	for (uint32_t ii = 0, num = hm.getMaxCapacity(); ii < num; ++ii)
	{
		ok = hm.insert(ii, uint16_t(ii) );
		REQUIRE(ok);
	}
}
