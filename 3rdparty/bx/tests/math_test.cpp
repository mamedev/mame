/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/math.h>

#if !BX_COMPILER_MSVC || BX_COMPILER_MSVC >= 1800
#include <cmath>
TEST_CASE("isFinite, isInfinite, isNan", "")
{
	for (uint64_t ii = 0; ii < UINT32_MAX; ii += rand()%(1<<13)+1)
	{
		union { uint32_t ui; float f; } u = { uint32_t(ii) };
		REQUIRE(std::isnan(u.f)    == bx::isNan(u.f) );
		REQUIRE(std::isfinite(u.f) == bx::isFinite(u.f) );
		REQUIRE(std::isinf(u.f)    == bx::isInfinite(u.f) );
	}
}
#endif // !BX_COMPILER_MSVC || BX_COMPILER_MSVC >= 1800


bool flog2_test(float _a)
{
	return bx::flog2(_a) == bx::flog(_a) * (1.0f / bx::flog(2.0f) );
}

TEST_CASE("flog2", "")
{
	flog2_test(0.0f);
	flog2_test(256.0f);
}

TEST_CASE("ToBits", "")
{
	REQUIRE(UINT32_C(0x12345678)         == bx::floatToBits( bx::bitsToFloat( UINT32_C(0x12345678) ) ) );
	REQUIRE(UINT64_C(0x123456789abcdef0) == bx::doubleToBits(bx::bitsToDouble(UINT32_C(0x123456789abcdef0) ) ) );
}

void mtxCheck(const float* _a, const float* _b)
{
	if (!bx::fequal(_a, _b, 16, 0.01f) )
	{
		DBG("\n"
			"A:\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"B:\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			"%10.4f %10.4f %10.4f %10.4f\n"
			, _a[ 0], _a[ 1], _a[ 2], _a[ 3]
			, _a[ 4], _a[ 5], _a[ 6], _a[ 7]
			, _a[ 8], _a[ 9], _a[10], _a[11]
			, _a[12], _a[13], _a[14], _a[15]
			, _b[ 0], _b[ 1], _b[ 2], _b[ 3]
			, _b[ 4], _b[ 5], _b[ 6], _b[ 7]
			, _b[ 8], _b[ 9], _b[10], _b[11]
			, _b[12], _b[13], _b[14], _b[15]
			);

		CHECK(false);
	}
}

TEST_CASE("quaternion", "")
{
	float mtxQ[16];
	float mtx[16];

	float quat[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	bx::mtxQuat(mtxQ, quat);
	bx::mtxIdentity(mtx);
	mtxCheck(mtxQ, mtx);

	float ax = bx::kPi/27.0f;
	float ay = bx::kPi/13.0f;
	float az = bx::kPi/7.0f;

	bx::quatRotateX(quat, ax);
	bx::mtxQuat(mtxQ, quat);
	bx::mtxRotateX(mtx, ax);
	mtxCheck(mtxQ, mtx);

	float euler[3];
	bx::quatToEuler(euler, quat);
	CHECK(bx::fequal(euler[0], ax, 0.001f) );

	bx::quatRotateY(quat, ay);
	bx::mtxQuat(mtxQ, quat);
	bx::mtxRotateY(mtx, ay);
	mtxCheck(mtxQ, mtx);

	bx::quatToEuler(euler, quat);
	CHECK(bx::fequal(euler[1], ay, 0.001f) );

	bx::quatRotateZ(quat, az);
	bx::mtxQuat(mtxQ, quat);
	bx::mtxRotateZ(mtx, az);
	mtxCheck(mtxQ, mtx);

	bx::quatToEuler(euler, quat);
	CHECK(bx::fequal(euler[2], az, 0.001f) );
}
