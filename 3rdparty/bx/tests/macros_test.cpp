/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/bx.h>
#include <bx/string.h>

BX_STATIC_ASSERT(false
	|| BX_CRT_BIONIC
	|| BX_CRT_GLIBC
	|| BX_CRT_LIBCXX
	|| BX_CRT_MINGW
	|| BX_CRT_MSVC
	|| BX_CRT_MUSL
	|| BX_CRT_NEWLIB
	);

BX_STATIC_ASSERT(1 == BX_VA_ARGS_COUNT(1) );
BX_STATIC_ASSERT(2 == BX_VA_ARGS_COUNT(1, 2) );
BX_STATIC_ASSERT(3 == BX_VA_ARGS_COUNT(1, 2, 3) );
BX_STATIC_ASSERT(4 == BX_VA_ARGS_COUNT(1, 2, 3, 4) );
BX_STATIC_ASSERT(5 == BX_VA_ARGS_COUNT(1, 2, 3, 4, 5) );
BX_STATIC_ASSERT(6 == BX_VA_ARGS_COUNT(1, 2, 3, 4, 5, 6) );

BX_STATIC_ASSERT(  0 == BX_ALIGN_16(  0) );
BX_STATIC_ASSERT( 16 == BX_ALIGN_16(  1) );
BX_STATIC_ASSERT( 16 == BX_ALIGN_16( 15) );
BX_STATIC_ASSERT( 16 == BX_ALIGN_16( 16) );
BX_STATIC_ASSERT(256 == BX_ALIGN_16(255) );

BX_STATIC_ASSERT(  0 == BX_ALIGN_256(  0) );
BX_STATIC_ASSERT(256 == BX_ALIGN_256(  1) );
BX_STATIC_ASSERT(256 == BX_ALIGN_256( 15) );
BX_STATIC_ASSERT(256 == BX_ALIGN_256(255) );
BX_STATIC_ASSERT(256 == BX_ALIGN_256(256) );
BX_STATIC_ASSERT(256 == BX_ALIGN_256(256) );
BX_STATIC_ASSERT(512 == BX_ALIGN_256(511) );

BX_NO_INLINE void unusedFunction()
{
	CHECK(false);
}

TEST(macros)
{
	uint32_t unused0;
	BX_UNUSED(unused0);

	uint32_t unused1;
	BX_UNUSED(unused0, unused1);

	uint32_t unused2;
	BX_UNUSED(unused0, unused1, unused2, unusedFunction() );

	CHECK_EQUAL(1, BX_VA_ARGS_COUNT(1) );
	CHECK_EQUAL(2, BX_VA_ARGS_COUNT(1, 2) );
	CHECK_EQUAL(3, BX_VA_ARGS_COUNT(1, 2, 3) );
	CHECK_EQUAL(4, BX_VA_ARGS_COUNT(1, 2, 3, 4) );
	CHECK_EQUAL(5, BX_VA_ARGS_COUNT(1, 2, 3, 4, 5) );
	CHECK_EQUAL(6, BX_VA_ARGS_COUNT(1, 2, 3, 4, 5, 6) );

	CHECK_EQUAL(0, bx::strCmp(BX_STRINGIZE(TEST 1234 %^&*), "TEST 1234 %^&*") );
}
