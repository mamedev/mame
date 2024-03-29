/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/bx.h>
#include <string.h>

namespace bx
{
	extern void memCopyRef(void* _dst, const void* _src, size_t _numBytes);
}

TEST_CASE("Bit cast round trip test using double to uint64_t", "[cast]")
{
	constexpr double d64v = 19880124.0;
	REQUIRE(sizeof(double) == sizeof(uint64_t) );

	SECTION("bx::bitCast")
	{
		const uint64_t u64v = bx::bitCast<uint64_t>(d64v);
		const double result = bx::bitCast<double>(u64v);
		REQUIRE(result == d64v);
	}

	SECTION("bx::memCopy")
	{
		uint64_t u64v = 0;
		double result = 0;
		bx::memCopy(&u64v, &d64v, sizeof(uint64_t) );
		bx::memCopy(&result, &u64v, sizeof(double) );
		REQUIRE(result == d64v);
	}

	SECTION("bx::memCopyRef")
	{
		uint64_t u64v = 0;
		double result = 0;
		bx::memCopyRef(&u64v, &d64v, sizeof(uint64_t) );
		bx::memCopyRef(&result, &u64v, sizeof(double) );
		REQUIRE(result == d64v);
	}

	SECTION("::memcpy")
	{
		uint64_t u64v = 0;
		double result = 0;
		::memcpy(&u64v, &d64v, sizeof(uint64_t) );
		::memcpy(&result, &u64v, sizeof(double) );
		REQUIRE(result == d64v);
	}
}

TEST_CASE("Bit cast round trip test using uint64_t to double", "[cast]")
{
	constexpr uint64_t u64v = 0x3fe9000000000000ull;
	REQUIRE(sizeof(uint64_t) == sizeof(double) );

	SECTION("bx::bitCast")
	{
		const double d64v = bx::bitCast<double>(u64v);
		const uint64_t result = bx::bitCast<uint64_t>(d64v);
		REQUIRE(result == u64v);
	}

	SECTION("bx::memCopy")
	{
		double d64v = 0;
		uint64_t result = 0;
		bx::memCopy(&d64v, &u64v, sizeof(double) );
		bx::memCopy(&result, &d64v, sizeof(uint64_t) );
		REQUIRE(result == u64v);
	}

	SECTION("bx::memCopyRef")
	{
		double d64v = 0;
		uint64_t result = 0;
		bx::memCopyRef(&d64v, &u64v, sizeof(double) );
		bx::memCopyRef(&result, &d64v, sizeof(uint64_t) );
		REQUIRE(result == u64v);
	}

	SECTION("::memcpy")
	{
		double d64v = 0;
		uint64_t result = 0;
		::memcpy(&d64v, &u64v, sizeof(double) );
		::memcpy(&result, &d64v, sizeof(uint64_t) );
		REQUIRE(result == u64v);
	}
}

TEST_CASE("Narrow cast", "[cast]")
{
	REQUIRE(127 == bx::narrowCast<int8_t>(int32_t(127) ) );
	REQUIRE_ASSERTS(bx::narrowCast<int8_t>(int32_t(128) ) );
	REQUIRE_ASSERTS(bx::narrowCast<int8_t>(uint32_t(128) ) );
	REQUIRE(128 == bx::narrowCast<uint8_t>(int32_t(128) ) );
}
