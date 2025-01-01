/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/bx.h>
#include <bx/sort.h>
#include <bx/string.h>
#include <bx/rng.h>

TEST_CASE("quickSort", "[sort]")
{
	const char* str[] =
	{
		"jabuka",
		"kruska",
		"malina",
		"jagoda",
	};

	REQUIRE(!bx::isSorted(str, BX_COUNTOF(str) ) );

	bx::quickSort(str, BX_COUNTOF(str) );

	REQUIRE(0 == bx::strCmp(str[0], "jabuka") );
	REQUIRE(0 == bx::strCmp(str[1], "jagoda") );
	REQUIRE(0 == bx::strCmp(str[2], "kruska") );
	REQUIRE(0 == bx::strCmp(str[3], "malina") );

	REQUIRE(bx::isSorted(str, BX_COUNTOF(str) ) );

	int8_t byte[128];
	bx::RngMwc rng;
	for (uint32_t ii = 0; ii < BX_COUNTOF(byte); ++ii)
	{
		byte[ii] = rng.gen()&0xff;
	}

	REQUIRE(!bx::isSorted(byte, BX_COUNTOF(byte) ) );

	bx::quickSort(byte, BX_COUNTOF(byte) );

	for (uint32_t ii = 1; ii < BX_COUNTOF(byte); ++ii)
	{
		REQUIRE(byte[ii-1] <= byte[ii]);
	}

	REQUIRE(bx::isSorted(byte, BX_COUNTOF(byte) ) );
}

TEST_CASE("binarySearch", "[sort]")
{
	const char* str[] =
	{
		"jabuka",
		"kruska",
		"malina",
		"jagoda",
	};

	REQUIRE(!bx::isSorted(str, BX_COUNTOF(str) ) );

	bx::quickSort(str, BX_COUNTOF(str) );
	REQUIRE(bx::isSorted(str, BX_COUNTOF(str) ) );

	auto bsearchStrCmpFn = [](const void* _lhs, const void* _rhs)
	{
		const char* lhs = (const char*)_lhs;
		const char* rhs = *(const char**)_rhs;
		return bx::strCmp(lhs, rhs);
	};

	REQUIRE(~4 == bx::binarySearch("sljiva", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 0 == bx::binarySearch("jabuka", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 1 == bx::binarySearch("jagoda", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 2 == bx::binarySearch("kruska", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 3 == bx::binarySearch("malina", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE(~3 == bx::binarySearch("kupina", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );

	REQUIRE( 0 == bx::lowerBound("jabuka", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 1 == bx::upperBound("jabuka", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );

	REQUIRE( 1 == bx::lowerBound("jagoda", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 2 == bx::upperBound("jagoda", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );

	REQUIRE( 2 == bx::lowerBound("kruska", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 3 == bx::upperBound("kruska", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );

	REQUIRE( 3 == bx::lowerBound("malina", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
	REQUIRE( 4 == bx::upperBound("malina", str, BX_COUNTOF(str), sizeof(str[0]), bsearchStrCmpFn) );
}

TEST_CASE("unique", "[sort]")
{
	//                   0    1    2    3    4    5    6    7    8    9   10   11   12   13 | 14
	int32_t test[] = { 100, 101, 101, 101, 103, 104, 105, 105, 105, 106, 106, 107, 108, 109 };
	REQUIRE(bx::isSorted(test, BX_COUNTOF(test) ) );

	REQUIRE(0 == bx::unique(test, 0) );
	REQUIRE(1 == bx::unique(test, 1) );

	REQUIRE(2 == bx::unique(test, 4) );
	bx::quickSort(test, BX_COUNTOF(test) );

	REQUIRE(3 == bx::unique(test, 5) );
	bx::quickSort(test, BX_COUNTOF(test) );

	uint32_t last = bx::unique(test, BX_COUNTOF(test) );
	REQUIRE(9 == last);

	REQUIRE(9 == bx::unique(test, last) );
}

TEST_CASE("lowerBound, upperBound int32_t", "[sort]")
{
	//                         0    1    2    3    4    5    6    7    8    9   10   11   12   13 | 14
	const int32_t test[] = { 100, 101, 101, 101, 103, 104, 105, 105, 105, 106, 106, 107, 108, 109 };
	REQUIRE(bx::isSorted(test, BX_COUNTOF(test) ) );

	const uint32_t resultLowerBound[] = { 0, 1, 4, 4, 5, 6,  9, 11, 12, 13 };
	const uint32_t resultUpperBound[] = { 1, 4, 4, 5, 6, 9, 11, 12, 13, 14 };

	STATIC_REQUIRE(10 == BX_COUNTOF(resultLowerBound) );
	STATIC_REQUIRE(10 == BX_COUNTOF(resultUpperBound) );

	for (int32_t key = test[0], keyMax = test[BX_COUNTOF(test)-1], ii = 0; key <= keyMax; ++key, ++ii)
	{
		REQUIRE(resultLowerBound[ii] == bx::lowerBound(key, test, BX_COUNTOF(test) ) );
		REQUIRE(resultUpperBound[ii] == bx::upperBound(key, test, BX_COUNTOF(test) ) );
	}
}

template<typename Ty>
int32_t compareAscendingTest(const Ty& _lhs, const Ty& _rhs)
{
	return bx::compareAscending<Ty>(&_lhs, &_rhs);
}

template<typename Ty>
int32_t compareDescendingTest(const Ty& _lhs, const Ty& _rhs)
{
	return bx::compareDescending<Ty>(&_lhs, &_rhs);
}

template<typename Ty>
void compareTest(const Ty& _min, const Ty& _max)
{
	REQUIRE(_min < _max);

	REQUIRE(-1 == compareAscendingTest<Ty>(bx::min<Ty>(), bx::max<Ty>() ) );
	REQUIRE(-1 == compareAscendingTest<Ty>(Ty(0),         bx::max<Ty>() ) );
	REQUIRE( 0 == compareAscendingTest<Ty>(bx::min<Ty>(), bx::min<Ty>() ) );
	REQUIRE( 0 == compareAscendingTest<Ty>(bx::max<Ty>(), bx::max<Ty>() ) );
	REQUIRE( 1 == compareAscendingTest<Ty>(bx::max<Ty>(), Ty(0)         ) );
	REQUIRE( 1 == compareAscendingTest<Ty>(bx::max<Ty>(), bx::min<Ty>() ) );

	REQUIRE(-1 == compareAscendingTest<Ty>(_min, _max) );
	REQUIRE( 0 == compareAscendingTest<Ty>(_min, _min) );
	REQUIRE( 0 == compareAscendingTest<Ty>(_max, _max) );
	REQUIRE( 1 == compareAscendingTest<Ty>(_max, _min) );

	REQUIRE( 1 == compareDescendingTest<Ty>(_min, _max) );
	REQUIRE( 0 == compareDescendingTest<Ty>(_min, _min) );
	REQUIRE( 0 == compareDescendingTest<Ty>(_max, _max) );
	REQUIRE(-1 == compareDescendingTest<Ty>(_max, _min) );
}

TEST_CASE("ComparisonFn", "[sort]")
{
	compareTest< int8_t>(  -13,   89);
	compareTest<int16_t>(-1389, 1389);
	compareTest<int32_t>(-1389, 1389);
	compareTest<int64_t>(-1389, 1389);

	compareTest< uint8_t>(  13,   89);
	compareTest<uint16_t>(  13, 1389);
	compareTest<uint32_t>(  13, 1389);
	compareTest<uint64_t>(  13, 1389);

	compareTest< float>(-13.89f, 1389.0f);
	compareTest<double>(-13.89f, 1389.0f);
}
