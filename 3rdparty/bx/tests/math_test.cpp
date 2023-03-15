/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/math.h>
#include <bx/file.h>

#include <math.h>
#include <stdint.h> // intXX_t
#include <limits.h> // UCHAR_*

#if !BX_COMPILER_MSVC || BX_COMPILER_MSVC >= 1800
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

bool log2_test(float _a)
{
	return bx::log2(_a) == bx::log(_a) * (1.0f / bx::log(2.0f) );
}

TEST_CASE("log2", "")
{
	log2_test(0.0f);
	log2_test(256.0f);

	REQUIRE(0.0f == bx::log2(1.0f) );
	REQUIRE(0 == bx::log2(1) );

	REQUIRE(1.0f == bx::log2(2.0f) );
	REQUIRE(1 == bx::log2(2) );

	REQUIRE(2.0f == bx::log2(4.0f) );
	REQUIRE(2 == bx::log2(4) );

	REQUIRE(3.0f == bx::log2(8.0f) );
	REQUIRE(3 == bx::log2(8) );

	REQUIRE(4.0f == bx::log2(16.0f) );
	REQUIRE(4 == bx::log2(16) );

	REQUIRE(5.0f == bx::log2(32.0f) );
	REQUIRE(5 == bx::log2(32) );

	REQUIRE(6.0f == bx::log2(64.0f) );
	REQUIRE(6 == bx::log2(64) );

	REQUIRE(7.0f == bx::log2(128.0f) );
	REQUIRE(7 == bx::log2(128) );

	REQUIRE(8.0f == bx::log2(256.0f) );
	REQUIRE(8 == bx::log2(256) );
}

TEST_CASE("libm", "")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	REQUIRE(1389.0f == bx::abs(-1389.0f) );
	REQUIRE(1389.0f == bx::abs( 1389.0f) );
	REQUIRE(   0.0f == bx::abs(-0.0f) );
	REQUIRE(   0.0f == bx::abs( 0.0f) );

	REQUIRE(389.0f == bx::mod(1389.0f, 1000.0f) );

	REQUIRE( 13.0f == bx::floor( 13.89f) );
	REQUIRE(-14.0f == bx::floor(-13.89f) );
	REQUIRE( 14.0f == bx::ceil(  13.89f) );
	REQUIRE(-13.0f == bx::ceil( -13.89f) );

	REQUIRE( 13.0f == bx::trunc( 13.89f) );
	REQUIRE(-13.0f == bx::trunc(-13.89f) );
	REQUIRE(bx::isEqual( 0.89f, bx::fract( 13.89f), 0.000001f) );
	REQUIRE(bx::isEqual(-0.89f, bx::fract(-13.89f), 0.000001f) );

	for (int32_t yy = -10; yy < 10; ++yy)
	{
		for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
		{
			bx::write(writer, &err, "ldexp(%f, %d) == %f (expected: %f)\n", xx, yy, bx::ldexp(xx, yy), ::ldexpf(xx, yy) );
			REQUIRE(bx::isEqual(bx::ldexp(xx, yy), ::ldexpf(xx, yy), 0.00001f) );
		}
	}

	for (float xx = -80.0f; xx < 80.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "exp(%f) == %f (expected: %f)\n", xx, bx::exp(xx), ::expf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::exp(xx), ::expf(xx), 0.00001f) );
	}

	for (float xx = 0.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "rsqrt(%f) == %f (expected: %f)\n", xx, bx::rsqrt(xx), 1.0f/::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::rsqrt(xx), 1.0f/::sqrtf(xx), 0.00001f) );
	}

	for (float xx = 0.0f; xx < 1000000.0f; xx += 1000.f)
	{
		bx::write(writer, &err, "sqrt(%f) == %f (expected: %f)\n", xx, bx::sqrt(xx), ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sqrt(xx), ::sqrtf(xx), 0.00001f) );
	}

	for (float xx = 0.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "sqrt(%f) == %f (expected: %f)\n", xx, bx::sqrt(xx), ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sqrt(xx), ::sqrtf(xx), 0.00001f) );
	}

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "pow(1.389f, %f) == %f (expected: %f)\n", xx, bx::pow(1.389f, xx), ::powf(1.389f, xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::pow(1.389f, xx), ::powf(1.389f, xx), 0.00001f) );
	}

	for (float xx = -1.0f; xx < 1.0f; xx += 0.001f)
	{
		bx::write(writer, &err, "asin(%f) == %f (expected: %f)\n", xx, bx::asin(xx), ::asinf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::asin(xx), ::asinf(xx), 0.0001f) );
	}

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "sin(%f) == %f (expected: %f)\n", xx, bx::sin(xx), ::sinf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sin(xx), ::sinf(xx), 0.00001f) );
	}

	for (float xx = -bx::kPi2; xx < bx::kPi2; xx += 0.0001f)
	{
		bx::write(writer, &err, "sin(%f) == %f (expected: %f)\n", xx, bx::sin(xx), ::sinf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sin(xx), ::sinf(xx), 0.00001f) );
	}

	for (float xx = -1.0f; xx < 1.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "sinh(%f) == %f (expected: %f)\n", xx, bx::sinh(xx), ::sinhf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sinh(xx), ::sinhf(xx), 0.00001f) );
	}

	for (float xx = -1.0f; xx < 1.0f; xx += 0.001f)
	{
		bx::write(writer, &err, "acos(%f) == %f (expected: %f\n)", xx, bx::acos(xx), ::acosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::acos(xx), ::acosf(xx), 0.0001f) );
	}

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "cos(%f) == %f (expected: %f)\n", xx, bx::cos(xx), ::cosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::cos(xx), ::cosf(xx), 0.00001f) );
	}

	for (float xx = -bx::kPi2; xx < bx::kPi2; xx += 0.0001f)
	{
		bx::write(writer, &err, "cos(%f) == %f (expected: %f)\n", xx, bx::cos(xx), ::cosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::cos(xx), ::cosf(xx), 0.00001f) );
	}

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "tan(%f) == %f (expected: %f)\n", xx, bx::tan(xx), ::tanf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::tan(xx), ::tanf(xx), 0.001f) );
	}

	for (float xx = -1.0f; xx < 1.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "tanh(%f) == %f (expected: %f\n", xx, bx::tanh(xx), ::tanhf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::tanh(xx), ::tanhf(xx), 0.00001f) );
	}

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "atan(%f) == %f (expected: %f)\n", xx, bx::atan(xx), ::atanf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::atan(xx), ::atanf(xx), 0.00001f) );
	}
}

TEST_CASE("atan2", "")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	REQUIRE(bx::isEqual(bx::atan2(0.0f,  0.0f), ::atan2f(0.0f,  0.0f), 0.00001f) );
	REQUIRE(bx::isEqual(bx::atan2(0.0f,  1.0f), ::atan2f(0.0f,  1.0f), 0.00001f) );
	REQUIRE(bx::isEqual(bx::atan2(0.0f, -1.0f), ::atan2f(0.0f, -1.0f), 0.00001f) );

	for (float yy = -100.0f; yy < 100.0f; yy += 0.1f)
	{
		for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
		{
			bx::write(writer, &err, "atan2(%f, %f) == %f (expected: %f)\n", yy, xx, bx::atan2(yy, xx), ::atan2f(yy, xx) );
			REQUIRE(err.isOk() );
			REQUIRE(bx::isEqual(bx::atan2(yy, xx), ::atan2f(yy, xx), 0.00001f) );
		}
	}
}

TEST_CASE("sign", "")
{
	REQUIRE(-1 == bx::sign(-0.1389f) );
	REQUIRE( 0 == bx::sign( 0.0000f) );
	REQUIRE( 1 == bx::sign( 0.1389f) );
}

TEST_CASE("ToBits", "")
{
	REQUIRE(UINT32_C(0x12345678)         == bx::floatToBits( bx::bitsToFloat( UINT32_C(0x12345678) ) ) );
	REQUIRE(UINT64_C(0x123456789abcdef0) == bx::doubleToBits(bx::bitsToDouble(UINT32_C(0x123456789abcdef0) ) ) );
}

TEST_CASE("lerp", "")
{
	REQUIRE(1389.0f == bx::lerp(1389.0f, 1453.0f, 0.0f) );
	REQUIRE(1453.0f == bx::lerp(1389.0f, 1453.0f, 1.0f) );
	REQUIRE(0.5f == bx::lerp(0.0f, 1.0f, 0.5f) );
}

void mtxCheck(const float* _a, const float* _b)
{
	if (!bx::isEqual(_a, _b, 16, 0.01f) )
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

		REQUIRE(false);
	}
}

TEST_CASE("quaternion", "")
{
	float mtxQ[16];
	float mtx[16];

	bx::Quaternion quat = bx::init::Identity;
	bx::Quaternion q2 = bx::init::None;

	bx::Vec3 axis = bx::init::None;
	bx::Vec3 euler = bx::init::None;
	float angle;

	bx::mtxFromQuaternion(mtxQ, quat);
	bx::mtxIdentity(mtx);
	mtxCheck(mtxQ, mtx);

	float ax = bx::kPi/27.0f;
	float ay = bx::kPi/13.0f;
	float az = bx::kPi/7.0f;

	{ // x
		quat = bx::rotateX(ax);
		bx::mtxFromQuaternion(mtxQ, quat);
		bx::mtxRotateX(mtx, ax);
		mtxCheck(mtxQ, mtx);

		bx::toAxisAngle(axis, angle, quat);
		REQUIRE(bx::isEqual(axis, bx::Vec3{1.0f, 0.0f, 0.0f}, 0.01f) );
		REQUIRE(bx::isEqual(angle, ax, 0.01f) );

		euler = bx::toEuler(quat);
		REQUIRE(bx::isEqual(euler.x, ax, 0.001f) );
		q2 = bx::fromEuler(euler);
		REQUIRE(bx::isEqual(quat, q2, 0.001f) );
	}

	{ // y
		quat = bx::rotateY(ay);
		bx::mtxFromQuaternion(mtxQ, quat);
		bx::mtxRotateY(mtx, ay);
		mtxCheck(mtxQ, mtx);

		bx::toAxisAngle(axis, angle, quat);
		REQUIRE(bx::isEqual(axis, bx::Vec3{0.0f, 1.0f, 0.0f}, 0.01f) );
		REQUIRE(bx::isEqual(angle, ay, 0.01f) );
		euler = bx::toEuler(quat);
		REQUIRE(bx::isEqual(euler.y, ay, 0.001f) );
		q2 = bx::fromEuler(euler);
		REQUIRE(bx::isEqual(quat, q2, 0.001f) );

	}

	{ // z
		quat = bx::rotateZ(az);
		bx::mtxFromQuaternion(mtxQ, quat);
		bx::mtxRotateZ(mtx, az);
		mtxCheck(mtxQ, mtx);

		bx::toAxisAngle(axis, angle, quat);
		REQUIRE(bx::isEqual(axis, bx::Vec3{0.0f, 0.0f, 1.0f}, 0.01f) );
		REQUIRE(bx::isEqual(angle, az, 0.01f) );

		euler = bx::toEuler(quat);
		REQUIRE(bx::isEqual(euler.z, az, 0.001f) );
		q2 = bx::fromEuler(euler);
		REQUIRE(bx::isEqual(quat, q2, 0.001f) );
	}
}

TEST_CASE("limits", "")
{
	STATIC_REQUIRE(bx::LimitsT<int8_t>::min == INT8_MIN);
	STATIC_REQUIRE(bx::LimitsT<int8_t>::max == INT8_MAX);

	STATIC_REQUIRE(bx::LimitsT<signed char>::min == CHAR_MIN);
	STATIC_REQUIRE(bx::LimitsT<signed char>::max == CHAR_MAX);

	STATIC_REQUIRE(bx::LimitsT<unsigned char>::min == 0);
	STATIC_REQUIRE(bx::LimitsT<unsigned char>::max == UCHAR_MAX);

	STATIC_REQUIRE(bx::LimitsT<int16_t>::min == INT16_MIN);
	STATIC_REQUIRE(bx::LimitsT<int16_t>::max == INT16_MAX);

	STATIC_REQUIRE(bx::LimitsT<uint16_t>::min == 0);
	STATIC_REQUIRE(bx::LimitsT<uint16_t>::max == UINT16_MAX);

	STATIC_REQUIRE(bx::LimitsT<int32_t>::min == INT32_MIN);
	STATIC_REQUIRE(bx::LimitsT<int32_t>::max == INT32_MAX);

	STATIC_REQUIRE(bx::LimitsT<uint32_t>::min == 0);
	STATIC_REQUIRE(bx::LimitsT<uint32_t>::max == UINT32_MAX);

	STATIC_REQUIRE(bx::LimitsT<int64_t>::min == INT64_MIN);
	STATIC_REQUIRE(bx::LimitsT<int64_t>::max == INT64_MAX);

	STATIC_REQUIRE(bx::LimitsT<uint64_t>::min == 0);
	STATIC_REQUIRE(bx::LimitsT<uint64_t>::max == UINT64_MAX);

	STATIC_REQUIRE(bx::LimitsT<float>::min == std::numeric_limits<float>::lowest() );
	STATIC_REQUIRE(bx::LimitsT<float>::max == std::numeric_limits<float>::max() );

	STATIC_REQUIRE(bx::LimitsT<double>::min == std::numeric_limits<double>::lowest() );
	STATIC_REQUIRE(bx::LimitsT<double>::max == std::numeric_limits<double>::max() );
}
