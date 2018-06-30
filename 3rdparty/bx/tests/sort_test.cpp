/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/sort.h>
#include <bx/string.h>
#include <bx/rng.h>

TEST_CASE("quickSort", "")
{
	const char* str[] =
	{
		"jabuka",
		"kruska",
		"malina",
		"jagoda",
	};

	bx::quickSort(str, BX_COUNTOF(str), sizeof(void*)
		, [](const void* _lhs, const void* _rhs)
		{
			const char* lhs = *(const char**)_lhs;
			const char* rhs = *(const char**)_rhs;
			return bx::strCmp(lhs, rhs);
		});

	REQUIRE(0 == bx::strCmp(str[0], "jabuka") );
	REQUIRE(0 == bx::strCmp(str[1], "jagoda") );
	REQUIRE(0 == bx::strCmp(str[2], "kruska") );
	REQUIRE(0 == bx::strCmp(str[3], "malina") );

	int8_t byte[128];
	bx::RngMwc rng;
	for (uint32_t ii = 0; ii < BX_COUNTOF(byte); ++ii)
	{
		byte[ii] = rng.gen()&0xff;
	}

	bx::quickSort(byte, BX_COUNTOF(byte), 1
		, [](const void* _lhs, const void* _rhs)
		{
			int8_t lhs = *(const int8_t*)_lhs;
			int8_t rhs = *(const int8_t*)_rhs;
			return lhs - rhs;
		});

	for (uint32_t ii = 1; ii < BX_COUNTOF(byte); ++ii)
	{
		REQUIRE(byte[ii-1] <= byte[ii]);
	}
}
