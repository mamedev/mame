/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/fpumath.h>

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

TEST(Quaternion)
{
	float mtxQ[16];
	float mtx[16];

	float quat[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	bx::mtxQuat(mtxQ, quat);
	bx::mtxIdentity(mtx);
	mtxCheck(mtxQ, mtx);

	float ax = bx::pi/27.0f;
	float ay = bx::pi/13.0f;
	float az = bx::pi/7.0f;

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
