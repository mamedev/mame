/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/bx.h>

TEST_CASE("Bit cast", "[cast]")
{
	STATIC_REQUIRE(0x4172f58bc0000000ull == bx::bitCast<uint64_t>(19880124.0) );
	STATIC_REQUIRE(0x3fe9000000000000ull == bx::bitCast<uint64_t>(0.781250) );
	STATIC_REQUIRE(19880124.0            == bx::bitCast<double>(0x4172f58bc0000000ull) );
	STATIC_REQUIRE(0.781250              == bx::bitCast<double>(0x3fe9000000000000ull) );
}

TEST_CASE("Narrow cast", "[cast]")
{
	REQUIRE(127 == bx::narrowCast<int8_t>(int32_t(127) ) );
	REQUIRE_ASSERTS(bx::narrowCast<int8_t>(int32_t(128) ) );
	REQUIRE_ASSERTS(bx::narrowCast<int8_t>(uint32_t(128) ) );
	REQUIRE(128 == bx::narrowCast<uint8_t>(int32_t(128) ) );
}
