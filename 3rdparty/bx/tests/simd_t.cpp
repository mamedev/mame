/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/simd_t.h>
#include <bx/fpumath.h>
#include <string.h>

using namespace bx;

union simd_cast
{
	bx::simd128_t f4;
	float f[4];
	uint32_t ui[4];
	int32_t i[4];
	char c[16];
};

void simd_check_bool(const char* _str, bool _a, bool _0)
{
	DBG("%s %d == %d"
		, _str
		, _a
		, _0
		);

	CHECK_EQUAL(_a, _0);
}

void simd_check_int32(const char* _str, bx::simd128_t _a, int32_t _0, int32_t _1, int32_t _2, int32_t _3)
{
	simd_cast c; c.f4 = _a;
	DBG("%s (%d, %d, %d, %d) == (%d, %d, %d, %d)"
		, _str
		, c.i[0], c.i[1], c.i[2], c.i[3]
		, _0, _1, _2, _3
		);

	CHECK_EQUAL(c.i[0], _0);
	CHECK_EQUAL(c.i[1], _1);
	CHECK_EQUAL(c.i[2], _2);
	CHECK_EQUAL(c.i[3], _3);
}

void simd_check_uint32(const char* _str, bx::simd128_t _a, uint32_t _0, uint32_t _1, uint32_t _2, uint32_t _3)
{
	simd_cast c; c.f4 = _a;

	DBG("%s (0x%08x, 0x%08x, 0x%08x, 0x%08x) == (0x%08x, 0x%08x, 0x%08x, 0x%08x)"
		, _str
		, c.ui[0], c.ui[1], c.ui[2], c.ui[3]
		, _0, _1, _2, _3
		);

	CHECK_EQUAL(c.ui[0], _0);
	CHECK_EQUAL(c.ui[1], _1);
	CHECK_EQUAL(c.ui[2], _2);
	CHECK_EQUAL(c.ui[3], _3);
}

void simd_check_float(const char* _str, bx::simd128_t _a, float _0, float _1, float _2, float _3)
{
	simd_cast c; c.f4 = _a;

	DBG("%s (%f, %f, %f, %f) == (%f, %f, %f, %f)"
		, _str
		, c.f[0], c.f[1], c.f[2], c.f[3]
		, _0, _1, _2, _3
		);

	CHECK(bx::fequal(c.f[0], _0, 0.0001f) );
	CHECK(bx::fequal(c.f[1], _1, 0.0001f) );
	CHECK(bx::fequal(c.f[2], _2, 0.0001f) );
	CHECK(bx::fequal(c.f[3], _3, 0.0001f) );
}

void simd_check_string(const char* _str, bx::simd128_t _a)
{
	simd_cast c; c.f4 = _a;
	const char test[5] = { c.c[0], c.c[4], c.c[8], c.c[12], '\0' };

	DBG("%s %s", _str, test);

	CHECK(0 == strcmp(_str, test) );
}

TEST(simd_swizzle)
{
	const simd128_t xyzw = simd_ild(0x78787878, 0x79797979, 0x7a7a7a7a, 0x77777777);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			simd_check_string("" #_x #_y #_z #_w "", simd_swiz_##_x##_y##_z##_w(xyzw) ); \

#include <bx/simd128_swizzle.inl>

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx
}

TEST(simd_shuffle)
{
	const simd128_t xyzw = simd_ild(0x78787878, 0x79797979, 0x7a7a7a7a, 0x77777777);
	const simd128_t ABCD = simd_ild(0x41414141, 0x42424242, 0x43434343, 0x44444444);
	simd_check_string("xyAB", simd_shuf_xyAB(xyzw, ABCD) );
	simd_check_string("ABxy", simd_shuf_ABxy(xyzw, ABCD) );
	simd_check_string("zwCD", simd_shuf_zwCD(xyzw, ABCD) );
	simd_check_string("CDzw", simd_shuf_CDzw(xyzw, ABCD) );
	simd_check_string("xAyB", simd_shuf_xAyB(xyzw, ABCD) );
	simd_check_string("zCwD", simd_shuf_zCwD(xyzw, ABCD) );
	simd_check_string("xAzC", simd_shuf_xAzC(xyzw, ABCD) );
	simd_check_string("yBwD", simd_shuf_yBwD(xyzw, ABCD) );
	simd_check_string("CzDw", simd_shuf_CzDw(xyzw, ABCD) );
}

TEST(simd_compare)
{
	simd_check_uint32("cmpeq"
		, simd_cmpeq(simd_ld(1.0f, 2.0f, 3.0f, 4.0f), simd_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0xffffffff, 0, 0
		);

	simd_check_uint32("cmplt"
		, simd_cmplt(simd_ld(1.0f, 2.0f, 3.0f, 4.0f), simd_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0, 0, 0
		);

	simd_check_uint32("cmple"
		, simd_cmple(simd_ld(1.0f, 2.0f, 3.0f, 4.0f), simd_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0xffffffff, 0, 0
		);

	simd_check_uint32("cmpgt"
		, simd_cmpgt(simd_ld(1.0f, 2.0f, 3.0f, 4.0f), simd_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0xffffffff, 0, 0xffffffff, 0xffffffff
		);

	simd_check_uint32("cmpge"
		, simd_cmpge(simd_ld(1.0f, 2.0f, 3.0f, 4.0f), simd_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
		);

	simd_check_uint32("icmpeq"
		, simd_icmpeq(simd_ild(0, 1, 2, 3), simd_ild(0, uint32_t(-2), 1, 3) )
		, 0xffffffff, 0, 0, 0xffffffff
		);

	simd_check_uint32("icmplt"
		, simd_icmplt(simd_ild(0, 1, 2, 3), simd_ild(0, uint32_t(-2), 1, 3) )
		, 0, 0, 0, 0
		);

	simd_check_uint32("icmpgt"
		, simd_icmpgt(simd_ild(0, 1, 2, 3), simd_ild(0, uint32_t(-2), 1, 3) )
		, 0, 0xffffffff, 0xffffffff, 0
		);
}

TEST(simd_test)
{
	simd_check_bool("test_any_xyzw"
		, simd_test_any_xyzw(simd_ild(0xffffffff, 0, 0, 0) )
		, true
		);

	simd_check_bool("test_all_xyzw"
		, simd_test_all_xyzw(simd_ild(0xffffffff, 0, 0xffffffff, 0) )
		, false
		);

	simd_check_bool("test_all_xyzw"
		, simd_test_all_xyzw(simd_ild(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff) )
		, true
		);

	simd_check_bool("test_all_xw"
		, simd_test_all_xw(simd_ild(0xffffffff, 0, 0, 0xffffffff) )
		, true
		);

	simd_check_bool("test_all_xzw"
		, simd_test_all_xzw(simd_ild(0xffffffff, 0, 0, 0xffffffff) )
		, false
		);
}

TEST(simd_load)
{
	simd_check_float("ld"
		, simd_ld(0.0f, 1.0f, 2.0f, 3.0f)
		, 0.0f, 1.0f, 2.0f, 3.0f
		);

	simd_check_int32("ild"
		, simd_ild(uint32_t(-1), 0, 1, 2)
		, uint32_t(-1), 0, 1, 2
		);

	simd_check_int32("ild"
		, simd_ild(uint32_t(-1), uint32_t(-2), uint32_t(-3), uint32_t(-4) )
		, uint32_t(-1), uint32_t(-2), uint32_t(-3), uint32_t(-4)
		);

	simd_check_uint32("zero", simd_zero()
		, 0, 0, 0, 0
		);

	simd_check_uint32("isplat", simd_isplat(0x80000001)
		, 0x80000001, 0x80000001, 0x80000001, 0x80000001
		);

	simd_check_float("isplat", simd_splat(1.0f)
		, 1.0f, 1.0f, 1.0f, 1.0f
		);
}

TEST(simd_arithmetic)
{
	simd_check_float("madd"
		, simd_madd(simd_ld(0.0f, 1.0f, 2.0f, 3.0f), simd_ld(4.0f, 5.0f, 6.0f, 7.0f), simd_ld(8.0f, 9.0f, 10.0f, 11.0f) )
		, 8.0f, 14.0f, 22.0f, 32.0f
		);

	simd_check_float("cross3"
		, simd_cross3(simd_ld(1.0f, 0.0f, 0.0f, 0.0f), simd_ld(0.0f, 1.0f, 0.0f, 0.0f) )
		, 0.0f, 0.0f, 1.0f, 0.0f
		);
}

TEST(simd_sqrt)
{
	simd_check_float("simd_sqrt"
		, simd_sqrt(simd_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);

	simd_check_float("simd_sqrt_nr_ni"
		, simd_sqrt_nr_ni(simd_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);

	simd_check_float("simd_sqrt_nr1_ni"
		, simd_sqrt_nr1_ni(simd_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);
}

TEST(float4)
{
	const simd128_t isplat = simd_isplat(0x80000001);
	simd_check_uint32("sll"
		, simd_sll(isplat, 1)
		, 0x00000002, 0x00000002, 0x00000002, 0x00000002
		);

	simd_check_uint32("srl"
		, simd_srl(isplat, 1)
		, 0x40000000, 0x40000000, 0x40000000, 0x40000000
		);

	simd_check_uint32("sra"
		, simd_sra(isplat, 1)
		, 0xc0000000, 0xc0000000, 0xc0000000, 0xc0000000
		);

	simd_check_uint32("and"
		, simd_and(simd_isplat(0x55555555), simd_isplat(0xaaaaaaaa) )
		, 0, 0, 0, 0
		);

	simd_check_uint32("or "
		, simd_or(simd_isplat(0x55555555), simd_isplat(0xaaaaaaaa) )
		, uint32_t(-1), uint32_t(-1), uint32_t(-1), uint32_t(-1)
		);

	simd_check_uint32("xor"
		, simd_or(simd_isplat(0x55555555), simd_isplat(0xaaaaaaaa) )
		, uint32_t(-1), uint32_t(-1), uint32_t(-1), uint32_t(-1)
		);

	simd_check_int32("imin"
		, simd_imin(simd_ild(0, 1, 2, 3), simd_ild(uint32_t(-1), 2, uint32_t(-2), 1) )
		, uint32_t(-1), 1, uint32_t(-2), 1
		);

	simd_check_float("min"
		, simd_min(simd_ld(0.0f, 1.0f, 2.0f, 3.0f), simd_ld(-1.0f, 2.0f, -2.0f, 1.0f) )
		, -1.0f, 1.0f, -2.0f, 1.0f
		);

	simd_check_int32("imax"
		, simd_imax(simd_ild(0, 1, 2, 3), simd_ild(uint32_t(-1), 2, uint32_t(-2), 1) )
		, 0, 2, 2, 3
		);

	simd_check_float("max"
		, simd_max(simd_ld(0.0f, 1.0f, 2.0f, 3.0f), simd_ld(-1.0f, 2.0f, -2.0f, 1.0f) )
		, 0.0f, 2.0f, 2.0f, 3.0f
		);
}
