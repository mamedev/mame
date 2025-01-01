/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/uint32_t.h>

TEST_CASE("StrideAlign", "[uint32_t]")
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

TEST_CASE("uint32_part", "[uint32_t]")
{
	REQUIRE(UINT32_C(0x55555555) == bx::uint32_part1by1(UINT16_MAX) );
	REQUIRE(UINT32_C(0x09249249) == bx::uint32_part1by2(0x3ff) );
}

TEST_CASE("uint32_gcd", "[uint32_t]")
{
	REQUIRE(1 == bx::uint32_gcd(13, 89) );
	REQUIRE(3 == bx::uint32_gcd( 3,  9) );
	REQUIRE(8 == bx::uint32_gcd( 8, 64) );
	REQUIRE(9 == bx::uint32_gcd(18, 81) );
}

TEST_CASE("uint32_lcm", "[uint32_t]")
{
	REQUIRE(1157 == bx::uint32_lcm(13, 89) );
	REQUIRE(   9 == bx::uint32_lcm( 3,  9) );
	REQUIRE(  48 == bx::uint32_lcm( 6, 16) );
	REQUIRE(  80 == bx::uint32_lcm(16, 20) );
}

TEST_CASE("halfTo/FromFloat", "[uint32_t]")
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

TEST_CASE("uint32_testpow2", "[uint32_t]")
{
	uint32_t shift = 0;
	uint32_t nextpow2 = bx::uint32_nextpow2(1);

	for (uint32_t ii = 1; ii < 1<<24; ++ii)
	{
		REQUIRE(nextpow2 == bx::uint32_nextpow2(ii) );

		if (bx::uint32_testpow2(ii) )
		{
			REQUIRE(ii == 1u << shift);
			++shift;

			REQUIRE(ii == nextpow2);
			nextpow2 = bx::uint32_nextpow2(ii+1);
		}
	}
}

TEST_CASE("uint32_roX", "[uint32_t]")
{
	REQUIRE(bx::uint32_rol(0x80000000, 1) == 1);
	REQUIRE(bx::uint32_ror(1, 1) == 0x80000000);
}

TEST_CASE("uint64_roX", "[uint32_t]")
{
	REQUIRE(bx::uint64_rol(0x8000000000000000, 1) == 1);
	REQUIRE(bx::uint64_ror(1, 1) == 0x8000000000000000);
}

TEST_CASE("align", "[uint32_t]")
{
	REQUIRE( bx::isAligned(0,  8) );
	REQUIRE(!bx::isAligned(7,  8) );
	REQUIRE( bx::isAligned(64, 8) );
	REQUIRE(!bx::isAligned(63, 8) );

	for (int32_t ii = 0; ii < 1024; ++ii)
	{
		REQUIRE(bx::isAligned(ii, 0) );
		REQUIRE(ii == bx::alignUp(ii, 0) );
		REQUIRE(ii == bx::alignDown(ii, 0) );
	}

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
