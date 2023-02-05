/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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

	REQUIRE(0  == bx::strideAlign<16>(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		REQUIRE(48 == bx::strideAlign<16>(ii+1, 12) );
	}

	uint32_t offset = 11;
	offset = bx::strideAlign(offset, 32);
	REQUIRE(offset == 32);

	offset = bx::strideAlign(offset, 24);
	REQUIRE(offset == 48);
}

TEST_CASE("uint32_cnt")
{
	REQUIRE( 0 == bx::uint32_cnttz<uint8_t >(1) );
	REQUIRE( 7 == bx::uint32_cnttz<uint8_t >(1<<7) );
	REQUIRE( 8 == bx::uint32_cnttz<uint8_t >(0) );
	REQUIRE( 1 == bx::uint32_cnttz<uint8_t >(0x3e) );
	REQUIRE( 0 == bx::uint32_cnttz<uint16_t>(1) );
	REQUIRE(15 == bx::uint32_cnttz<uint16_t>(1<<15) );
	REQUIRE(16 == bx::uint32_cnttz<uint16_t>(0) );
	REQUIRE( 0 == bx::uint32_cnttz<uint32_t>(1) );
	REQUIRE(32 == bx::uint32_cnttz<uint32_t>(0) );
	REQUIRE(31 == bx::uint32_cnttz<uint32_t>(1u<<31) );
	REQUIRE( 0 == bx::uint32_cnttz<uint64_t>(1) );
	REQUIRE(64 == bx::uint32_cnttz<uint64_t>(0) );

	REQUIRE( 7 == bx::uint32_cntlz<uint8_t >(1) );
	REQUIRE( 8 == bx::uint32_cntlz<uint8_t >(0) );
	REQUIRE( 2 == bx::uint32_cntlz<uint8_t >(0x3e) );
	REQUIRE(15 == bx::uint32_cntlz<uint16_t>(1) );
	REQUIRE(16 == bx::uint32_cntlz<uint16_t>(0) );
	REQUIRE(31 == bx::uint32_cntlz<uint32_t>(1) );
	REQUIRE(32 == bx::uint32_cntlz<uint32_t>(0) );
	REQUIRE(63 == bx::uint32_cntlz<uint64_t>(1) );
	REQUIRE(64 == bx::uint32_cntlz<uint64_t>(0) );

	REQUIRE( 0 == bx::uint32_cntbits(0) );
	REQUIRE( 1 == bx::uint32_cntbits(1) );

	REQUIRE( 4 == bx::uint32_cntbits<uint8_t>(0x55) );
	REQUIRE( 8 == bx::uint32_cntbits<uint16_t>(0x5555) );
	REQUIRE(16 == bx::uint32_cntbits<uint32_t>(0x55555555) );
	REQUIRE(32 == bx::uint32_cntbits<uint64_t>(0x5555555555555555) );

	REQUIRE( 8 == bx::uint32_cntbits(UINT8_MAX) );
	REQUIRE(16 == bx::uint32_cntbits(UINT16_MAX) );
	REQUIRE(32 == bx::uint32_cntbits(UINT32_MAX) );
	REQUIRE(64 == bx::uint32_cntbits(UINT64_MAX) );
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

TEST_CASE("uint32_testpow2", "")
{
	uint32_t shift = 0;

	for (uint32_t ii = 0; ii < UINT32_MAX; ++ii)
	{
		if (bx::uint32_testpow2(ii) )
		{
			REQUIRE(ii == 1u << shift);
			++shift;
		}
	}
}

TEST_CASE("uint32_roX", "")
{
	REQUIRE(bx::uint32_rol(0x80000000, 1) == 1);
	REQUIRE(bx::uint32_ror(1, 1) == 0x80000000);
}

TEST_CASE("uint64_roX", "")
{
	REQUIRE(bx::uint64_rol(0x8000000000000000, 1) == 1);
	REQUIRE(bx::uint64_ror(1, 1) == 0x8000000000000000);
}

TEST_CASE("align", "")
{
	REQUIRE( bx::isAligned(0,  8) );
	REQUIRE(!bx::isAligned(7,  8) );
	REQUIRE( bx::isAligned(64, 8) );
	REQUIRE(!bx::isAligned(63, 8) );

	REQUIRE(  0 == bx::alignUp(  0, 16) );
	REQUIRE( 16 == bx::alignUp(  1, 16) );
	REQUIRE( 16 == bx::alignUp( 15, 16) );
	REQUIRE( 16 == bx::alignUp( 16, 16) );
	REQUIRE(256 == bx::alignUp(255, 16) );
	REQUIRE(  0 == bx::alignUp(-1,  16)  );
	REQUIRE(-16 == bx::alignUp(-31, 16) );

	REQUIRE(  0 == bx::alignUp(  0, 256) );
	REQUIRE(256 == bx::alignUp(  1, 256) );
	REQUIRE(256 == bx::alignUp( 15, 256) );
	REQUIRE(256 == bx::alignUp(255, 256) );
	REQUIRE(256 == bx::alignUp(256, 256) );
	REQUIRE(256 == bx::alignUp(256, 256) );
	REQUIRE(512 == bx::alignUp(511, 256) );

	REQUIRE(  0 == bx::alignDown(  0, 16) );
	REQUIRE(  0 == bx::alignDown(  1, 16) );
	REQUIRE(  0 == bx::alignDown( 15, 16) );
	REQUIRE( 16 == bx::alignDown( 16, 16) );
	REQUIRE(240 == bx::alignDown(255, 16) );
	REQUIRE(-16 == bx::alignDown(-1,  16)  );
	REQUIRE(-32 == bx::alignDown(-31, 16) );
}
