/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/math.h>
#include <bx/file.h>

#include <math.h>
#include <stdint.h> // intXX_t
#include <limits.h> // UCHAR_*

TEST_CASE("isFinite, isInfinite, isNan", "[math]")
{
	for (uint64_t ii = 0; ii < UINT32_MAX; ii += rand()%(1<<13)+1)
	{
		union { uint32_t ui; float f; } u = { uint32_t(ii) };

#if BX_PLATFORM_OSX
		REQUIRE(::__isnanf(u.f)    == bx::isNan(u.f) );
		REQUIRE(::__isfinitef(u.f) == bx::isFinite(u.f) );
		REQUIRE(::__isinff(u.f)    == bx::isInfinite(u.f) );
#elif BX_COMPILER_MSVC
		REQUIRE(!!::isnan(u.f)    == bx::isNan(u.f) );
		REQUIRE(!!::isfinite(u.f) == bx::isFinite(u.f) );
		REQUIRE(!!::isinf(u.f)    == bx::isInfinite(u.f) );
#elif !BX_CRT_MINGW
		REQUIRE(::isnanf(u.f)  == bx::isNan(u.f) );
		REQUIRE(::finitef(u.f) == bx::isFinite(u.f) );
		REQUIRE(::isinff(u.f)  == bx::isInfinite(u.f) );
#endif // BX_*
	}
}

bool log2_test(float _a)
{
	return bx::log2(_a) == bx::log(_a) * (1.0f / bx::log(2.0f) );
}

TEST_CASE("log2", "[math][libm]")
{
	log2_test(0.0f);
	log2_test(256.0f);

	REQUIRE(0.0f == bx::log2(1.0f) );
	REQUIRE(1.0f == bx::log2(2.0f) );
	REQUIRE(2.0f == bx::log2(4.0f) );
	REQUIRE(3.0f == bx::log2(8.0f) );
	REQUIRE(4.0f == bx::log2(16.0f) );
	REQUIRE(5.0f == bx::log2(32.0f) );
	REQUIRE(6.0f == bx::log2(64.0f) );
	REQUIRE(7.0f == bx::log2(128.0f) );
	REQUIRE(8.0f == bx::log2(256.0f) );
}

TEST_CASE("ceilLog2", "[math]")
{
	REQUIRE(0 == bx::ceilLog2(-1) );
	REQUIRE(0 == bx::ceilLog2(0) );
	REQUIRE(0 == bx::ceilLog2(1) );
	REQUIRE(1 == bx::ceilLog2(2) );
	REQUIRE(2 == bx::ceilLog2(4) );
	REQUIRE(3 == bx::ceilLog2(8) );
	REQUIRE(4 == bx::ceilLog2(16) );
	REQUIRE(5 == bx::ceilLog2(32) );
	REQUIRE(6 == bx::ceilLog2(64) );
	REQUIRE(7 == bx::ceilLog2(128) );
	REQUIRE(8 == bx::ceilLog2(256) );

	{
		uint32_t ii = 0;
		for (; ii < 8; ++ii)
		{
			REQUIRE(ii == bx::ceilLog2(uint8_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint16_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 16; ++ii)
		{
			REQUIRE(ii == bx::ceilLog2(uint16_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 32; ++ii)
		{
			REQUIRE(ii == bx::ceilLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::ceilLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 64; ++ii)
		{
			REQUIRE(ii == bx::ceilLog2(uint64_t(1llu<<ii) ) );
		}
	}

	for (uint32_t ii = 1; ii < INT32_MAX; ii += rand()%(1<<13)+1)
	{
//		DBG("%u: %u %u", ii, bx::uint32_nextpow2(ii), bx::nextPow2(ii) );
		REQUIRE(bx::nextPow2(ii) == bx::uint32_nextpow2(ii) );
	}
}

TEST_CASE("floorLog2", "[math]")
{
	REQUIRE(0 == bx::floorLog2(-1) );
	REQUIRE(0 == bx::floorLog2(0) );
	REQUIRE(0 == bx::floorLog2(1) );
	REQUIRE(1 == bx::floorLog2(2) );
	REQUIRE(2 == bx::floorLog2(4) );
	REQUIRE(3 == bx::floorLog2(8) );
	REQUIRE(4 == bx::floorLog2(16) );
	REQUIRE(5 == bx::floorLog2(32) );
	REQUIRE(6 == bx::floorLog2(64) );
	REQUIRE(7 == bx::floorLog2(128) );
	REQUIRE(8 == bx::floorLog2(256) );

	{
		uint32_t ii = 0;
		for (; ii < 8; ++ii)
		{
			REQUIRE(ii == bx::floorLog2(uint8_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint16_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 16; ++ii)
		{
			REQUIRE(ii == bx::floorLog2(uint16_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 32; ++ii)
		{
			REQUIRE(ii == bx::floorLog2(uint32_t(1<<ii) ) );
			REQUIRE(ii == bx::floorLog2(uint64_t(1llu<<ii) ) );
		}

		for (; ii < 64; ++ii)
		{
			REQUIRE(ii == bx::floorLog2(uint64_t(1llu<<ii) ) );
		}
	}
}

TEST_CASE("ceilLog2 & floorLog2", "[math]")
{
	{
		uint32_t prev = 0;
		uint32_t next = 0;
		for (uint32_t ii = 0; ii < (1<<18); ++ii)
		{
			if (bx::isPowerOf2(ii) )
			{
				REQUIRE(bx::ceilLog2(ii) == bx::floorLog2(ii) );
				prev = next;
				++next;
			}
			else
			{
				REQUIRE(prev == bx::floorLog2(ii) );
				REQUIRE(next == bx::ceilLog2(ii) );
			}
		}
	}
}

TEST_CASE("countTrailingZeros", "[math]")
{
	REQUIRE( 0 == bx::countTrailingZeros<uint8_t >(1) );
	REQUIRE( 7 == bx::countTrailingZeros<uint8_t >(1<<7) );
	REQUIRE( 8 == bx::countTrailingZeros<uint8_t >(0) );
	REQUIRE( 1 == bx::countTrailingZeros<uint8_t >(0x3e) );
	REQUIRE( 0 == bx::countTrailingZeros<uint16_t>(1) );
	REQUIRE(15 == bx::countTrailingZeros<uint16_t>(1<<15) );
	REQUIRE(16 == bx::countTrailingZeros<uint16_t>(0) );
	REQUIRE( 0 == bx::countTrailingZeros<uint32_t>(1) );
	REQUIRE(32 == bx::countTrailingZeros<uint32_t>(0) );
	REQUIRE(31 == bx::countTrailingZeros<uint32_t>(1u<<31) );
	REQUIRE( 0 == bx::countTrailingZeros<uint64_t>(1) );
	REQUIRE(64 == bx::countTrailingZeros<uint64_t>(0) );
}

TEST_CASE("countLeadingZeros", "[math]")
{
	REQUIRE( 7 == bx::countLeadingZeros<uint8_t >(1) );
	REQUIRE( 8 == bx::countLeadingZeros<uint8_t >(0) );
	REQUIRE( 2 == bx::countLeadingZeros<uint8_t >(0x3e) );
	REQUIRE(15 == bx::countLeadingZeros<uint16_t>(1) );
	REQUIRE(16 == bx::countLeadingZeros<uint16_t>(0) );
	REQUIRE(31 == bx::countLeadingZeros<uint32_t>(1) );
	REQUIRE(32 == bx::countLeadingZeros<uint32_t>(0) );
	REQUIRE(63 == bx::countLeadingZeros<uint64_t>(1) );
	REQUIRE(64 == bx::countLeadingZeros<uint64_t>(0) );
}

TEST_CASE("countBits", "[math]")
{
	REQUIRE( 0 == bx::countBits(0) );
	REQUIRE( 1 == bx::countBits(1) );

	REQUIRE( 4 == bx::countBits<uint8_t>(0x55) );
	REQUIRE( 8 == bx::countBits<uint16_t>(0x5555) );
	REQUIRE(16 == bx::countBits<uint32_t>(0x55555555) );
	REQUIRE(32 == bx::countBits<uint64_t>(0x5555555555555555) );

	REQUIRE( 8 == bx::countBits(UINT8_MAX) );
	REQUIRE(16 == bx::countBits(UINT16_MAX) );
	REQUIRE(32 == bx::countBits(UINT32_MAX) );
	REQUIRE(64 == bx::countBits(UINT64_MAX) );
}

TEST_CASE("findFirstSet", "[math]")
{
	REQUIRE( 1 == bx::findFirstSet<uint8_t >(1) );
	REQUIRE( 8 == bx::findFirstSet<uint8_t >(1<<7) );
	REQUIRE( 0 == bx::findFirstSet<uint8_t >(0) );
	REQUIRE( 2 == bx::findFirstSet<uint8_t >(0x3e) );
	REQUIRE( 1 == bx::findFirstSet<uint16_t>(1) );
	REQUIRE(16 == bx::findFirstSet<uint16_t>(1<<15) );
	REQUIRE( 0 == bx::findFirstSet<uint16_t>(0) );
	REQUIRE( 1 == bx::findFirstSet<uint32_t>(1) );
	REQUIRE( 0 == bx::findFirstSet<uint32_t>(0) );
	REQUIRE(32 == bx::findFirstSet<uint32_t>(1u<<31) );
	REQUIRE( 1 == bx::findFirstSet<uint64_t>(1) );
	REQUIRE( 0 == bx::findFirstSet<uint64_t>(0) );
}

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4723) // potential divide by 0

TEST_CASE("rcp", "[math][libm]")
{
	REQUIRE(1.0f == bx::rcp(1.0f) );
	REQUIRE(2.0f == bx::rcp(0.5f) );
	REQUIRE(bx::isInfinite(bx::rcp( 0.0f) ) );
	REQUIRE(bx::isInfinite(bx::rcp(-0.0f) ) );
}

TEST_CASE("rcpSafe", "[math][libm]")
{
	REQUIRE(1.0f == bx::rcpSafe(1.0f) );
	REQUIRE(2.0f == bx::rcpSafe(0.5f) );
	REQUIRE(!bx::isInfinite(bx::rcpSafe( 0.0f) ) );
	REQUIRE(!bx::isInfinite(bx::rcpSafe(-0.0f) ) );
}

TEST_CASE("rsqrt", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	// rsqrtRef
	REQUIRE(bx::isInfinite(bx::rsqrtRef(0.0f) ) );

	for (float xx = bx::kNearZero; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "rsqrtRef(%f) == %f (expected: %f)\n", xx, bx::rsqrtRef(xx), 1.0f / ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::rsqrtRef(xx), 1.0f / ::sqrtf(xx), 0.00001f) );
	}

	// rsqrtSimd
	REQUIRE(bx::isInfinite(bx::rsqrtSimd(0.0f) ) );

	for (float xx = bx::kNearZero; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "rsqrtSimd(%f) == %f (expected: %f)\n", xx, bx::rsqrtSimd(xx), 1.0f / ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::rsqrtSimd(xx), 1.0f / ::sqrtf(xx), 0.00001f) );
	}

	// rsqrt
	REQUIRE(bx::isInfinite(1.0f / ::sqrtf(0.0f) ) );
	REQUIRE(bx::isInfinite(bx::rsqrt(0.0f) ) );

	for (float xx = bx::kNearZero; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "rsqrt(%f) == %f (expected: %f)\n", xx, bx::rsqrt(xx), 1.0f / ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::rsqrt(xx), 1.0f / ::sqrtf(xx), 0.00001f) );
	}
}

TEST_CASE("sqrt", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	// sqrtRef
	REQUIRE(bx::isNan(bx::sqrtRef(-1.0f) ) );
	REQUIRE(bx::isEqual(bx::sqrtRef(0.0f), ::sqrtf(0.0f), 0.0f) );
	REQUIRE(bx::isEqual(bx::sqrtRef(1.0f), ::sqrtf(1.0f), 0.0f) );

	for (float xx = 0.0f; xx < 1000000.0f; xx += 1000.f)
	{
		bx::write(writer, &err, "sqrtRef(%f) == %f (expected: %f)\n", xx, bx::sqrtRef(xx), ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sqrtRef(xx), ::sqrtf(xx), 0.00001f) );
	}

	// sqrtSimd
	REQUIRE(bx::isNan(bx::sqrtSimd(-1.0f) ) );
	REQUIRE(bx::isEqual(bx::sqrtSimd(0.0f), ::sqrtf(0.0f), 0.00001f) );
	REQUIRE(bx::isEqual(bx::sqrtSimd(1.0f), ::sqrtf(1.0f), 0.00001f) );

	for (float xx = 0.0f; xx < 1000000.0f; xx += 1000.f)
	{
		bx::write(writer, &err, "sqrtSimd(%f) == %f (expected: %f)\n", xx, bx::sqrtSimd(xx), ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sqrtSimd(xx), ::sqrtf(xx), 0.00001f) );
	}

	for (float xx = 0.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "sqrt(%f) == %f (expected: %f)\n", xx, bx::sqrt(xx), ::sqrtf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sqrt(xx), ::sqrtf(xx), 0.00001f) );
	}

	// sqrt
	REQUIRE(bx::isNan(::sqrtf(-1.0f) ) );
	REQUIRE(bx::isNan(bx::sqrt(-1.0f) ) );
	REQUIRE(bx::isEqual(bx::sqrt(0.0f), ::sqrtf(0.0f), 0.00001f) );
	REQUIRE(bx::isEqual(bx::sqrt(1.0f), ::sqrtf(1.0f), 0.00001f) );

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
}

BX_PRAGMA_DIAGNOSTIC_POP();

TEST_CASE("abs", "[math][libm]")
{
	REQUIRE(1389.0f == bx::abs(-1389.0f) );
	REQUIRE(1389.0f == bx::abs( 1389.0f) );
	REQUIRE(   0.0f == bx::abs(-0.0f) );
	REQUIRE(   0.0f == bx::abs( 0.0f) );
}

TEST_CASE("mod", "[math][libm]")
{
	REQUIRE(389.0f == bx::mod(1389.0f, 1000.0f) );
	REQUIRE( 89.0f == bx::mod(1389.0f, 100.0f) );
	REQUIRE(  9.0f == bx::mod(1389.0f, 10.0f) );
	REQUIRE(  4.0f == bx::mod(1389.0f, 5.0f) );
	REQUIRE(  1.0f == bx::mod(1389.0f, 2.0f) );
}

TEST_CASE("floor", "[math][libm]")
{
	REQUIRE( 13.0f == bx::floor( 13.89f) );
	REQUIRE(-14.0f == bx::floor(-13.89f) );
}

TEST_CASE("ceil", "[math][libm]")
{
	REQUIRE( 14.0f == bx::ceil(  13.89f) );
	REQUIRE(-13.0f == bx::ceil( -13.89f) );
}

TEST_CASE("trunc", "[math][libm]")
{
	REQUIRE( 13.0f == bx::trunc( 13.89f) );
	REQUIRE(-13.0f == bx::trunc(-13.89f) );
}

TEST_CASE("fract", "[math][libm]")
{
	REQUIRE(bx::isEqual( 0.89f, bx::fract( 13.89f), 0.000001f) );
	REQUIRE(bx::isEqual(-0.89f, bx::fract(-13.89f), 0.000001f) );
}

TEST_CASE("ldexp", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (int32_t yy = -10; yy < 10; ++yy)
	{
		for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
		{
			bx::write(writer, &err, "ldexp(%f, %d) == %f (expected: %f)\n", xx, yy, bx::ldexp(xx, yy), ::ldexpf(xx, yy) );
			REQUIRE(bx::isEqual(bx::ldexp(xx, yy), ::ldexpf(xx, yy), 0.00001f) );
		}
	}
}

TEST_CASE("exp", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -80.0f; xx < 80.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "exp(%f) == %f (expected: %f)\n", xx, bx::exp(xx), ::expf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::exp(xx), ::expf(xx), 0.00001f) );
	}
}

TEST_CASE("pow", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "pow(1.389f, %f) == %f (expected: %f)\n", xx, bx::pow(1.389f, xx), ::powf(1.389f, xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::pow(1.389f, xx), ::powf(1.389f, xx), 0.00001f) );
	}
}

TEST_CASE("asin", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -1.0f; xx < 1.0f; xx += 0.001f)
	{
		bx::write(writer, &err, "asin(%f) == %f (expected: %f)\n", xx, bx::asin(xx), ::asinf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::asin(xx), ::asinf(xx), 0.0001f) );
	}
}

TEST_CASE("sin", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

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
}

TEST_CASE("sinCos", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		float ss, cc;
		bx::sinCosApprox(ss, cc, xx);

		bx::write(writer, &err, "sinCos(%f) == sin %f (expected: %f)\n", xx, ss, ::sinf(xx) );
		bx::write(writer, &err, "sinCos(%f) == cos %f (expected: %f)\n", xx, cc, ::cosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(ss, ::sinf(xx), 0.001f) );
		REQUIRE(bx::isEqual(cc, ::cosf(xx), 0.00001f) );
	}

	for (float xx = -bx::kPi2; xx < bx::kPi2; xx += 0.0001f)
	{
		float ss, cc;
		bx::sinCosApprox(ss, cc, xx);

		bx::write(writer, &err, "sinCos(%f) == sin %f (expected: %f)\n", xx, ss, ::sinf(xx) );
		bx::write(writer, &err, "sinCos(%f) == cos %f (expected: %f)\n", xx, cc, ::cosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(ss, ::sinf(xx), 0.001f) );
		REQUIRE(bx::isEqual(cc, ::cosf(xx), 0.00001f) );
	}
}

TEST_CASE("sinh", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -1.0f; xx < 1.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "sinh(%f) == %f (expected: %f)\n", xx, bx::sinh(xx), ::sinhf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::sinh(xx), ::sinhf(xx), 0.00001f) );
	}
}

TEST_CASE("acos", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -1.0f; xx < 1.0f; xx += 0.001f)
	{
		bx::write(writer, &err, "acos(%f) == %f (expected: %f\n)", xx, bx::acos(xx), ::acosf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::acos(xx), ::acosf(xx), 0.0001f) );
	}
}

TEST_CASE("cos", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

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
}

TEST_CASE("tan", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "tan(%f) == %f (expected: %f)\n", xx, bx::tan(xx), ::tanf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::tan(xx), ::tanf(xx), 0.001f) );
	}
}

TEST_CASE("tanh", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -1.0f; xx < 1.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "tanh(%f) == %f (expected: %f\n", xx, bx::tanh(xx), ::tanhf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::tanh(xx), ::tanhf(xx), 0.00001f) );
	}
}

TEST_CASE("atan", "[math][libm]")
{
	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	for (float xx = -100.0f; xx < 100.0f; xx += 0.1f)
	{
		bx::write(writer, &err, "atan(%f) == %f (expected: %f)\n", xx, bx::atan(xx), ::atanf(xx) );
		REQUIRE(err.isOk() );
		REQUIRE(bx::isEqual(bx::atan(xx), ::atanf(xx), 0.00001f) );
	}
}

TEST_CASE("atan2", "[math][libm]")
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

TEST_CASE("sign", "[math][libm]")
{
	STATIC_REQUIRE(-1 == bx::sign(-0.1389f) );
	STATIC_REQUIRE( 0 == bx::sign( 0.0000f) );
	STATIC_REQUIRE( 1 == bx::sign( 0.1389f) );

	REQUIRE(-1 == bx::sign(-bx::kFloatInfinity) );
	REQUIRE( 1 == bx::sign( bx::kFloatInfinity) );
}

TEST_CASE("signBit", "[math][libm]")
{
	STATIC_REQUIRE( bx::signBit(-0.1389f) );
	STATIC_REQUIRE(!bx::signBit( 0.0000f) );
	STATIC_REQUIRE(!bx::signBit( 0.1389f) );

	REQUIRE( bx::signBit(-bx::kFloatInfinity) );
	REQUIRE(!bx::signBit( bx::kFloatInfinity) );
}

TEST_CASE("copySign", "[math][libm]")
{
	STATIC_REQUIRE( 0.1389f == bx::copySign(-0.1389f, +1389) );
	STATIC_REQUIRE(-0.0000f == bx::copySign( 0.0000f, -1389) );
	STATIC_REQUIRE(-0.1389f == bx::copySign( 0.1389f, -1389) );

	REQUIRE(-bx::kFloatInfinity == bx::copySign(bx::kFloatInfinity, -1389) );
}

TEST_CASE("bitsToFloat, floatToBits, bitsToDouble, doubleToBits", "[math]")
{
	REQUIRE(UINT32_C(0x12345678)         == bx::floatToBits( bx::bitsToFloat( UINT32_C(0x12345678) ) ) );
	REQUIRE(UINT64_C(0x123456789abcdef0) == bx::doubleToBits(bx::bitsToDouble(UINT32_C(0x123456789abcdef0) ) ) );
}

TEST_CASE("lerp", "[math]")
{
	REQUIRE(1389.0f == bx::lerp(1389.0f, 1453.0f, 0.0f) );
	REQUIRE(1453.0f == bx::lerp(1389.0f, 1453.0f, 1.0f) );
	REQUIRE(   0.5f == bx::lerp(   0.0f,    1.0f, 0.5f) );
	REQUIRE(   0.0f == bx::lerp(   0.0f,    0.0f, 0.5f) );
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

TEST_CASE("vec3", "[math][vec3]")
{
	REQUIRE(bx::isEqual({0.0f, 0.0f, 0.0f}, bx::normalize({0.0f, 0.0f, 0.0f}), 0.0f) );

	bx::Vec3 normalized = bx::normalize({0.0f, 1.0f, 0.0f});
	REQUIRE(bx::isEqual(normalized, {0.0f, 1.0f, 0.0f}, 0.00001f) );

	float length = bx::length(normalized);
	REQUIRE(bx::isEqual(length, 1.0f, 0.00001f) );
}

TEST_CASE("quaternion", "[math][quaternion]")
{
	float mtxQ[16];
	float mtx[16];

	bx::Quaternion quat = bx::InitIdentity;
	bx::Quaternion q2 = bx::InitNone;

	bx::Vec3 axis = bx::InitNone;
	bx::Vec3 euler = bx::InitNone;
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

TEST_CASE("limits", "[math]")
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

	STATIC_REQUIRE(bx::kFloatSmallest  == std::numeric_limits<float>::min() );
	STATIC_REQUIRE(bx::kDoubleSmallest == std::numeric_limits<double>::min() );
}
