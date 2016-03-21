/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/float4_t.h>
#include <bx/fpumath.h>
#include <string.h>

using namespace bx;

union float4_cast
{
	bx::float4_t f4;
	float f[4];
	uint32_t ui[4];
	int32_t i[4];
	char c[16];
};

void float4_check_bool(const char* _str, bool _a, bool _0)
{
	DBG("%s %d == %d"
		, _str
		, _a
		, _0
		);

	CHECK_EQUAL(_a, _0);
}

void float4_check_int32(const char* _str, bx::float4_t _a, int32_t _0, int32_t _1, int32_t _2, int32_t _3)
{
	float4_cast c; c.f4 = _a;
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

void float4_check_uint32(const char* _str, bx::float4_t _a, uint32_t _0, uint32_t _1, uint32_t _2, uint32_t _3)
{
	float4_cast c; c.f4 = _a;

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

void float4_check_float(const char* _str, bx::float4_t _a, float _0, float _1, float _2, float _3)
{
	float4_cast c; c.f4 = _a;

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

void float4_check_string(const char* _str, bx::float4_t _a)
{
	float4_cast c; c.f4 = _a;
	const char test[5] = { c.c[0], c.c[4], c.c[8], c.c[12], '\0' };

	DBG("%s %s", _str, test);

	CHECK(0 == strcmp(_str, test) );
}

TEST(float4_swizzle)
{
	const float4_t xyzw = float4_ild(0x78787878, 0x79797979, 0x7a7a7a7a, 0x77777777);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			float4_check_string("" #_x #_y #_z #_w "", float4_swiz_##_x##_y##_z##_w(xyzw) ); \

#include <bx/float4_swizzle.inl>

#undef IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx
}

TEST(float4_shuffle)
{
	const float4_t xyzw = float4_ild(0x78787878, 0x79797979, 0x7a7a7a7a, 0x77777777);
	const float4_t ABCD = float4_ild(0x41414141, 0x42424242, 0x43434343, 0x44444444);
	float4_check_string("xyAB", float4_shuf_xyAB(xyzw, ABCD) );
	float4_check_string("ABxy", float4_shuf_ABxy(xyzw, ABCD) );
	float4_check_string("zwCD", float4_shuf_zwCD(xyzw, ABCD) );
	float4_check_string("CDzw", float4_shuf_CDzw(xyzw, ABCD) );
	float4_check_string("xAyB", float4_shuf_xAyB(xyzw, ABCD) );
	float4_check_string("zCwD", float4_shuf_zCwD(xyzw, ABCD) );
	float4_check_string("xAzC", float4_shuf_xAzC(xyzw, ABCD) );
	float4_check_string("yBwD", float4_shuf_yBwD(xyzw, ABCD) );
	float4_check_string("CzDw", float4_shuf_CzDw(xyzw, ABCD) );
}

TEST(float4_compare)
{
	float4_check_uint32("cmpeq"
		, float4_cmpeq(float4_ld(1.0f, 2.0f, 3.0f, 4.0f), float4_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0xffffffff, 0, 0
		);

	float4_check_uint32("cmplt"
		, float4_cmplt(float4_ld(1.0f, 2.0f, 3.0f, 4.0f), float4_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0, 0, 0
		);

	float4_check_uint32("cmple"
		, float4_cmple(float4_ld(1.0f, 2.0f, 3.0f, 4.0f), float4_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0, 0xffffffff, 0, 0
		);

	float4_check_uint32("cmpgt"
		, float4_cmpgt(float4_ld(1.0f, 2.0f, 3.0f, 4.0f), float4_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0xffffffff, 0, 0xffffffff, 0xffffffff
		);

	float4_check_uint32("cmpge"
		, float4_cmpge(float4_ld(1.0f, 2.0f, 3.0f, 4.0f), float4_ld(0.0f, 2.0f, 0.0f, 3.0f) )
		, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
		);

	float4_check_uint32("icmpeq"
		, float4_icmpeq(float4_ild(0, 1, 2, 3), float4_ild(0, uint32_t(-2), 1, 3) )
		, 0xffffffff, 0, 0, 0xffffffff
		);

	float4_check_uint32("icmplt"
		, float4_icmplt(float4_ild(0, 1, 2, 3), float4_ild(0, uint32_t(-2), 1, 3) )
		, 0, 0, 0, 0
		);

	float4_check_uint32("icmpgt"
		, float4_icmpgt(float4_ild(0, 1, 2, 3), float4_ild(0, uint32_t(-2), 1, 3) )
		, 0, 0xffffffff, 0xffffffff, 0
		);
}

TEST(float4_test)
{
	float4_check_bool("test_any_xyzw"
		, float4_test_any_xyzw(float4_ild(0xffffffff, 0, 0, 0) )
		, true
		);

	float4_check_bool("test_all_xyzw"
		, float4_test_all_xyzw(float4_ild(0xffffffff, 0, 0xffffffff, 0) )
		, false
		);

	float4_check_bool("test_all_xyzw"
		, float4_test_all_xyzw(float4_ild(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff) )
		, true
		);

	float4_check_bool("test_all_xw"
		, float4_test_all_xw(float4_ild(0xffffffff, 0, 0, 0xffffffff) )
		, true
		);

	float4_check_bool("test_all_xzw"
		, float4_test_all_xzw(float4_ild(0xffffffff, 0, 0, 0xffffffff) )
		, false
		);
}

TEST(float4_load)
{
	float4_check_float("ld"
		, float4_ld(0.0f, 1.0f, 2.0f, 3.0f)
		, 0.0f, 1.0f, 2.0f, 3.0f
		);

	float4_check_int32("ild"
		, float4_ild(uint32_t(-1), 0, 1, 2)
		, uint32_t(-1), 0, 1, 2
		);

	float4_check_int32("ild"
		, float4_ild(uint32_t(-1), uint32_t(-2), uint32_t(-3), uint32_t(-4) )
		, uint32_t(-1), uint32_t(-2), uint32_t(-3), uint32_t(-4)
		);

	float4_check_uint32("zero", float4_zero()
		, 0, 0, 0, 0
		);

	float4_check_uint32("isplat", float4_isplat(0x80000001)
		, 0x80000001, 0x80000001, 0x80000001, 0x80000001
		);

	float4_check_float("isplat", float4_splat(1.0f)
		, 1.0f, 1.0f, 1.0f, 1.0f
		);
}

TEST(float4_arithmetic)
{
	float4_check_float("madd"
		, float4_madd(float4_ld(0.0f, 1.0f, 2.0f, 3.0f), float4_ld(4.0f, 5.0f, 6.0f, 7.0f), float4_ld(8.0f, 9.0f, 10.0f, 11.0f) )
		, 8.0f, 14.0f, 22.0f, 32.0f
		);

	float4_check_float("cross3"
		, float4_cross3(float4_ld(1.0f, 0.0f, 0.0f, 0.0f), float4_ld(0.0f, 1.0f, 0.0f, 0.0f) )
		, 0.0f, 0.0f, 1.0f, 0.0f
		);
}

TEST(float4_sqrt)
{
	float4_check_float("float4_sqrt"
		, float4_sqrt(float4_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);

	float4_check_float("float4_sqrt_nr_ni"
		, float4_sqrt_nr_ni(float4_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);

	float4_check_float("float4_sqrt_nr1_ni"
		, float4_sqrt_nr1_ni(float4_ld(1.0f, 16.0f, 65536.0f, 123456.0f) )
		, 1.0f, 4.0f, 256.0f, 351.363060096f
		);
}

TEST(float4)
{
	const float4_t isplat = float4_isplat(0x80000001);
	float4_check_uint32("sll"
		, float4_sll(isplat, 1)
		, 0x00000002, 0x00000002, 0x00000002, 0x00000002
		);

	float4_check_uint32("srl"
		, float4_srl(isplat, 1)
		, 0x40000000, 0x40000000, 0x40000000, 0x40000000
		);

	float4_check_uint32("sra"
		, float4_sra(isplat, 1)
		, 0xc0000000, 0xc0000000, 0xc0000000, 0xc0000000
		);

	float4_check_uint32("and"
		, float4_and(float4_isplat(0x55555555), float4_isplat(0xaaaaaaaa) )
		, 0, 0, 0, 0
		);

	float4_check_uint32("or "
		, float4_or(float4_isplat(0x55555555), float4_isplat(0xaaaaaaaa) )
		, uint32_t(-1), uint32_t(-1), uint32_t(-1), uint32_t(-1)
		);

	float4_check_uint32("xor"
		, float4_or(float4_isplat(0x55555555), float4_isplat(0xaaaaaaaa) )
		, uint32_t(-1), uint32_t(-1), uint32_t(-1), uint32_t(-1)
		);

	float4_check_int32("imin"
		, float4_imin(float4_ild(0, 1, 2, 3), float4_ild(uint32_t(-1), 2, uint32_t(-2), 1) )
		, uint32_t(-1), 1, uint32_t(-2), 1
		);

	float4_check_float("min"
		, float4_min(float4_ld(0.0f, 1.0f, 2.0f, 3.0f), float4_ld(-1.0f, 2.0f, -2.0f, 1.0f) )
		, -1.0f, 1.0f, -2.0f, 1.0f
		);

	float4_check_int32("imax"
		, float4_imax(float4_ild(0, 1, 2, 3), float4_ild(uint32_t(-1), 2, uint32_t(-2), 1) )
		, 0, 2, 2, 3
		);

	float4_check_float("max"
		, float4_max(float4_ld(0.0f, 1.0f, 2.0f, 3.0f), float4_ld(-1.0f, 2.0f, -2.0f, 1.0f) )
		, 0.0f, 2.0f, 2.0f, 3.0f
		);
}
