/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/uint32_t.h>

TEST_CASE("StrideAlign")
{
	REQUIRE(0  == bx::strideAlign(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		REQUIRE(12 == bx::strideAlign(ii+1, 12) );
	}

	REQUIRE(0  == bx::strideAlign16(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		REQUIRE(48 == bx::strideAlign16(ii+1, 12) );
	}
}

TEST_CASE("uint32_cnt")
{
	REQUIRE( 0 == bx::uint32_cnttz(UINT32_C(1) ) );
	REQUIRE( 0 == bx::uint32_cnttz_ref(UINT32_C(1) ) );

	REQUIRE(31 == bx::uint32_cntlz(UINT32_C(1) ) );
	REQUIRE(31 == bx::uint32_cntlz_ref(UINT32_C(1) ) );

	REQUIRE( 0 == bx::uint64_cnttz(UINT64_C(1) ) );
	REQUIRE( 0 == bx::uint64_cnttz_ref(UINT64_C(1) ) );

	REQUIRE(63 == bx::uint64_cntlz(UINT64_C(1) ) );
	REQUIRE(63 == bx::uint64_cntlz_ref(UINT64_C(1) ) );

	REQUIRE( 1 == bx::uint32_cntbits(1) );
	REQUIRE( 1 == bx::uint32_cntbits_ref(1) );

	REQUIRE(16 == bx::uint32_cntbits(UINT16_MAX) );
	REQUIRE(16 == bx::uint32_cntbits_ref(UINT16_MAX) );

	REQUIRE(32 == bx::uint32_cntbits(UINT32_MAX) );
	REQUIRE(32 == bx::uint32_cntbits_ref(UINT32_MAX) );
}

TEST_CASE("uint32_part")
{
	REQUIRE(UINT32_C(0x55555555) == bx::uint32_part1by1(UINT16_MAX) );
	REQUIRE(UINT32_C(0x09249249) == bx::uint32_part1by2(0x3ff) );
}

TEST_CASE("halfTo/FromFloat", "")
{
	for (uint32_t ii = 0; ii < 0x7c00; ++ii)
	{
		const uint16_t orig = uint16_t(ii);
		const float    htf = bx::halfToFloat(orig);
		const uint16_t hff = bx::halfFromFloat(htf);
		REQUIRE(orig == hff);
	}

	for (uint32_t ii = 0x8000; ii < 0xfc00; ++ii)
	{
		const uint16_t orig = uint16_t(ii);
		const float    htf = bx::halfToFloat(orig);
		const uint16_t hff = bx::halfFromFloat(htf);
		REQUIRE(orig == hff);
	}
}
