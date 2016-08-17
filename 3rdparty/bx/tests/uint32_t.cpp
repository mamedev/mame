/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/uint32_t.h>

TEST(StrideAlign)
{
	CHECK(0  == bx::strideAlign(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		CHECK(12 == bx::strideAlign(ii+1, 12) );
	}

	CHECK(0  == bx::strideAlign16(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		CHECK(48 == bx::strideAlign16(ii+1, 12) );
	}
}

TEST(uint32_cnt)
{
	CHECK( 0 == bx::uint32_cnttz(UINT32_C(1) ) );
	CHECK( 0 == bx::uint32_cnttz_ref(UINT32_C(1) ) );

	CHECK(31 == bx::uint32_cntlz(UINT32_C(1) ) );
	CHECK(31 == bx::uint32_cntlz_ref(UINT32_C(1) ) );

	CHECK( 0 == bx::uint64_cnttz(UINT64_C(1) ) );
	CHECK( 0 == bx::uint64_cnttz_ref(UINT64_C(1) ) );

	CHECK(63 == bx::uint64_cntlz(UINT64_C(1) ) );
	CHECK(63 == bx::uint64_cntlz_ref(UINT64_C(1) ) );

	CHECK( 1 == bx::uint32_cntbits(1) );
	CHECK( 1 == bx::uint32_cntbits_ref(1) );

	CHECK(16 == bx::uint32_cntbits(UINT16_MAX) );
	CHECK(16 == bx::uint32_cntbits_ref(UINT16_MAX) );

	CHECK(32 == bx::uint32_cntbits(UINT32_MAX) );
	CHECK(32 == bx::uint32_cntbits_ref(UINT32_MAX) );
}

TEST(uint32_part)
{
	CHECK(UINT32_C(0x55555555) == bx::uint32_part1by1(UINT16_MAX) );
	CHECK(UINT32_C(0x09249249) == bx::uint32_part1by2(0x3ff) );
}
