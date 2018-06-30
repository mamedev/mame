/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/filepath.h>
#include <bx/string.h>
#include <bx/handlealloc.h>
#include <bx/sort.h>

bx::AllocatorI* g_allocator;

TEST_CASE("chars", "")
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
	{
		REQUIRE(!bx::isLower(ch) );
		REQUIRE(!bx::isNumeric(ch) );
		REQUIRE(bx::isUpper(ch) );
		REQUIRE(bx::isAlpha(ch) );
		REQUIRE(bx::isAlphaNum(ch) );
		REQUIRE(bx::isLower(bx::toLower(ch) ) );
	}
}

TEST_CASE("strLen", "")
{
	const char* test = "test";

	REQUIRE(0 == bx::strLen(test, 0) );
	REQUIRE(2 == bx::strLen(test, 2) );
	REQUIRE(4 == bx::strLen(test, INT32_MAX) );
}

TEST_CASE("strCopy", "")
{
	char dst[128];
	size_t num;

	num = bx::strCopy(dst, 1, "blah");
	REQUIRE(num == 0);

	num = bx::strCopy(dst, 3, "blah", 3);
	REQUIRE(0 == bx::strCmp(dst, "bl") );
	REQUIRE(num == 2);

	num = bx::strCopy(dst, sizeof(dst), "blah", 3);
	REQUIRE(0 == bx::strCmp(dst, "bla") );
	REQUIRE(num == 3);

	num = bx::strCopy(dst, sizeof(dst), "blah");
	REQUIRE(0 == bx::strCmp(dst, "bl", 2) );
	REQUIRE(0 == bx::strCmp(dst, "blah") );
	REQUIRE(num == 4);
}

TEST_CASE("strCat", "")
{
	char dst[128] = { '\0' };

	REQUIRE(0 == bx::strCat(dst, 1, "cat") );

	REQUIRE(4 == bx::strCopy(dst, 5, "copy") );
	REQUIRE(3 == bx::strCat(dst, 8, "cat") );
	REQUIRE(0 == bx::strCmp(dst, "copycat") );
	REQUIRE(0 == bx::strCmp(dst, "copy", 4) );

	REQUIRE(1 == bx::strCat(dst, BX_COUNTOF(dst), "------", 1) );
	REQUIRE(3 == bx::strCat(dst, BX_COUNTOF(dst), "cat") );
	REQUIRE(0 == bx::strCmp(dst, "copycat-cat") );
}

TEST_CASE("strCmp", "")
{
	REQUIRE(0 != bx::strCmp("meh", "meh/") );
}

TEST_CASE("strCmpI", "")
{
	REQUIRE(0 == bx::strCmpI("test", "test") );
	REQUIRE(0 == bx::strCmpI("test", "testestes", 4) );
	REQUIRE(0 == bx::strCmpI("testestes", "test", 4) );
	REQUIRE(0 != bx::strCmpI("preprocess", "platform") );

	const char* abvgd = "abvgd";
	const char* abvgx = "abvgx";
	const char* empty = "";
	REQUIRE(0 == bx::strCmpI(abvgd, abvgd) );
	REQUIRE(0 == bx::strCmpI(abvgd, abvgx, 4) );
	REQUIRE(0 == bx::strCmpI(empty, empty) );

	REQUIRE(0 >  bx::strCmpI(abvgd, abvgx) );
	REQUIRE(0 >  bx::strCmpI(empty, abvgd) );

	REQUIRE(0 <  bx::strCmpI(abvgx, abvgd) );
	REQUIRE(0 <  bx::strCmpI(abvgd, empty) );
}

TEST_CASE("strCmpV", "")
{
	REQUIRE(0 == bx::strCmpV("test", "test") );
	REQUIRE(0 == bx::strCmpV("test", "testestes", 4) );
	REQUIRE(0 == bx::strCmpV("testestes", "test", 4) );
	REQUIRE(0 != bx::strCmpV("preprocess", "platform") );

	const char* abvgd = "abvgd";
	const char* abvgx = "abvgx";
	const char* empty = "";
	REQUIRE(0 == bx::strCmpV(abvgd, abvgd) );
	REQUIRE(0 == bx::strCmpV(abvgd, abvgx, 4) );
	REQUIRE(0 == bx::strCmpV(empty, empty) );

	REQUIRE(0 >  bx::strCmpV(abvgd, abvgx) );
	REQUIRE(0 >  bx::strCmpV(empty, abvgd) );

	REQUIRE(0 <  bx::strCmpV(abvgx, abvgd) );
	REQUIRE(0 <  bx::strCmpV(abvgd, empty) );
}

static int32_t strCmpV(const void* _lhs, const void* _rhs)
{
	const char* lhs = *(const char**)_lhs;
	const char* rhs = *(const char**)_rhs;
	int32_t result = bx::strCmpV(lhs, rhs);
	return result;
}

TEST_CASE("strCmpV sort", "")
{
	const char* test[] =
	{
		"test_1.txt",
		"test_10.txt",
		"test_100.txt",
		"test_15.txt",
		"test_11.txt",
		"test_23.txt",
		"test_3.txt",
	};

	const char* expected[] =
	{
		"test_1.txt",
		"test_3.txt",
		"test_10.txt",
		"test_11.txt",
		"test_15.txt",
		"test_23.txt",
		"test_100.txt",
	};

	BX_STATIC_ASSERT(BX_COUNTOF(test) == BX_COUNTOF(expected) );

	bx::quickSort(test, BX_COUNTOF(test), sizeof(const char*), strCmpV);

	for (uint32_t ii = 0; ii < BX_COUNTOF(test); ++ii)
	{
		REQUIRE(0 == bx::strCmp(test[ii], expected[ii]) );
	}
}

TEST_CASE("strRFind", "")
{
	const char* test = "test";
	REQUIRE(NULL == bx::strRFind(bx::StringView(test, 0), 's') );
	REQUIRE(NULL == bx::strRFind(bx::StringView(test, 1), 's') );
	REQUIRE(&test[2] == bx::strRFind(test, 's') );
}

TEST_CASE("strFindI", "")
{
	const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

	REQUIRE(NULL == bx::strFindI(bx::StringView(test, 8), "quick") );
	REQUIRE(NULL == bx::strFindI(test, "quick1") );
	REQUIRE(&test[4] == bx::strFindI(bx::StringView(test, 9), "quick") );
	REQUIRE(&test[4] == bx::strFindI(test, "quick") );
}

TEST_CASE("strFind", "")
{
	{
		const char* test = "test";

		REQUIRE(NULL == bx::strFind(bx::StringView(test, 0), 's') );
		REQUIRE(NULL == bx::strFind(bx::StringView(test, 2), 's') );
		REQUIRE(&test[2] == bx::strFind(test, 's') );
	}

	{
		const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

		REQUIRE(NULL == bx::strFind(bx::StringView(test, 8), "quick") );
		REQUIRE(NULL == bx::strFind(test, "quick1") );
		REQUIRE(NULL == bx::strFind(bx::StringView(test, 9), "quick") );
		REQUIRE(NULL == bx::strFind(test, "quick") );

		REQUIRE(NULL == bx::strFind(bx::StringView(test, 8), "Quick") );
		REQUIRE(NULL == bx::strFind(test, "Quick1") );
		REQUIRE(&test[4] == bx::strFind(bx::StringView(test, 9), "Quick") );
		REQUIRE(&test[4] == bx::strFind(test, "Quick") );

		REQUIRE(NULL == bx::strFind("vgd", 'a') );
	}
}

template<typename Ty>
static bool testToString(Ty _value, const char* _expected)
{
	char tmp[1024];
	int32_t num = bx::toString(tmp, BX_COUNTOF(tmp), _value);
	int32_t len = (int32_t)bx::strLen(_expected);
	if (0 == bx::strCmp(tmp, _expected)
	&&  num == len)
	{
		return true;
	}

	printf("result '%s' (%d), expected '%s' (%d)\n", tmp, num, _expected, len);
	return false;
}

TEST_CASE("toString int32_t/uint32_t", "")
{
	REQUIRE(testToString(0,          "0") );
	REQUIRE(testToString(-256,       "-256") );
	REQUIRE(testToString(INT32_MAX,  "2147483647") );
	REQUIRE(testToString(UINT32_MAX, "4294967295") );
}

TEST_CASE("toString double", "")
{
	REQUIRE(testToString(0.0,                     "0.0") );
	REQUIRE(testToString(-0.0,                    "-0.0") );
	REQUIRE(testToString(1.0,                     "1.0") );
	REQUIRE(testToString(-1.0,                    "-1.0") );
	REQUIRE(testToString(1.2345,                  "1.2345") );
	REQUIRE(testToString(1.2345678,               "1.2345678") );
	REQUIRE(testToString(0.123456789012,          "0.123456789012") );
	REQUIRE(testToString(1234567.8,               "1234567.8") );
	REQUIRE(testToString(-79.39773355813419,      "-79.39773355813419") );
	REQUIRE(testToString(0.000001,                "0.000001") );
	REQUIRE(testToString(0.0000001,               "1e-7") );
	REQUIRE(testToString(1e30,                    "1e30") );
	REQUIRE(testToString(1.234567890123456e30,    "1.234567890123456e30") );
	REQUIRE(testToString(-5e-324,                 "-5e-324") );
	REQUIRE(testToString(2.225073858507201e-308,  "2.225073858507201e-308") );
	REQUIRE(testToString(2.2250738585072014e-308, "2.2250738585072014e-308") );
	REQUIRE(testToString(1.7976931348623157e308,  "1.7976931348623157e308") );
	REQUIRE(testToString(0.00000123123123,        "0.00000123123123") );
	REQUIRE(testToString(0.000000123123123,       "1.23123123e-7") );
	REQUIRE(testToString(123123.123,              "123123.123") );
	REQUIRE(testToString(1231231.23,              "1231231.23") );
	REQUIRE(testToString(0.000000000123123,       "1.23123e-10") );
	REQUIRE(testToString(0.0000000001,            "1e-10") );
	REQUIRE(testToString(-270.000000,             "-270.0") );
}

static bool testFromString(double _value, const char* _input)
{
	char tmp[1024];
	bx::toString(tmp, BX_COUNTOF(tmp), _value);

	double lhs;
	bx::fromString(&lhs, tmp);

	double rhs;
	bx::fromString(&rhs, _input);

	if (lhs == rhs)
	{
		return true;
	}

	printf("result '%f', input '%s'\n", _value, _input);
	return false;
}

TEST_CASE("fromString double", "")
{
	REQUIRE(testFromString(0.0,                     "0.0") );
	REQUIRE(testFromString(-0.0,                    "-0.0") );
	REQUIRE(testFromString(1.0,                     "1.0") );
	REQUIRE(testFromString(-1.0,                    "-1.0") );
	REQUIRE(testFromString(1.2345,                  "1.2345") );
	REQUIRE(testFromString(1.2345678,               "1.2345678") );
	REQUIRE(testFromString(0.123456789012,          "0.123456789012") );
	REQUIRE(testFromString(1234567.8,               "1234567.8") );
	REQUIRE(testFromString(-79.39773355813419,      "-79.39773355813419") );
	REQUIRE(testFromString(0.000001,                "0.000001") );
	REQUIRE(testFromString(0.0000001,               "1e-7") );
	REQUIRE(testFromString(1e30,                    "1e30") );
	REQUIRE(testFromString(1.234567890123456e30,    "1.234567890123456e30") );
	REQUIRE(testFromString(-5e-324,                 "-5e-324") );
	REQUIRE(testFromString(2.225073858507201e-308,  "2.225073858507201e-308") );
	REQUIRE(testFromString(2.2250738585072014e-308, "2.2250738585072014e-308") );
	REQUIRE(testFromString(1.7976931348623157e308,  "1.7976931348623157e308") );
	REQUIRE(testFromString(0.00000123123123,        "0.00000123123123") );
	REQUIRE(testFromString(0.000000123123123,       "1.23123123e-7") );
	REQUIRE(testFromString(123123.123,              "123123.123") );
	REQUIRE(testFromString(1231231.23,              "1231231.23") );
	REQUIRE(testFromString(0.000000000123123,       "1.23123e-10") );
	REQUIRE(testFromString(0.0000000001,            "1e-10") );
	REQUIRE(testFromString(-270.000000,             "-270.0") );
}

static bool testFromString(int32_t _value, const char* _input)
{
	char tmp[1024];
	bx::toString(tmp, BX_COUNTOF(tmp), _value);

	double lhs;
	bx::fromString(&lhs, tmp);

	double rhs;
	bx::fromString(&rhs, _input);

	if (lhs == rhs)
	{
		return true;
	}

	printf("result '%d', input '%s'\n", _value, _input);
	return false;
}

TEST_CASE("fromString int32_t", "")
{
	REQUIRE(testFromString(1389,   "1389") );
	REQUIRE(testFromString(1389,   "  1389") );
	REQUIRE(testFromString(1389,   "+1389") );
	REQUIRE(testFromString(-1389,  "-1389") );
	REQUIRE(testFromString(-1389,  " -1389") );
	REQUIRE(testFromString(555333, "555333") );
	REQUIRE(testFromString(-21,    "-021") );
}

TEST_CASE("StringView", "")
{
	bx::StringView sv("test");
	REQUIRE(4 == sv.getLength() );

	bx::DefaultAllocator crt;
	g_allocator = &crt;

	typedef bx::StringT<&g_allocator> String;

	String st(sv);
	REQUIRE(4 == st.getLength() );

	st.append("test");
	REQUIRE(8 == st.getLength() );

	st.append(bx::StringView("test", 2) );
	REQUIRE(10 == st.getLength() );

	REQUIRE(0 == bx::strCmp(st.getPtr(), "testtestte") );

	st.clear();
	REQUIRE(0 == st.getLength() );
	REQUIRE(4 == sv.getLength() );

	st.append("test");
	REQUIRE(4 == st.getLength() );

	sv.clear();
	REQUIRE(0 == sv.getLength() );
}

TEST_CASE("Trim", "")
{
	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "ab"), "vgd") );
	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "vagbd"), "") );
	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "vgd"), "abvgd") );
	REQUIRE(0 == bx::strCmp(bx::strLTrim("/555333/podmac/", "/"), "555333/podmac/") );

	REQUIRE(0 == bx::strCmp(bx::strRTrim("abvgd", "vagbd"), "") );
	REQUIRE(0 == bx::strCmp(bx::strRTrim("abvgd", "abv"), "abvgd") );
	REQUIRE(0 == bx::strCmp(bx::strRTrim("/555333/podmac/", "/"), "/555333/podmac") );

	REQUIRE(0 == bx::strCmp(bx::strTrim("abvgd", "da"), "bvg") );
	REQUIRE(0 == bx::strCmp(bx::strTrim("<1389>", "<>"), "1389") );
	REQUIRE(0 == bx::strCmp(bx::strTrim("/555333/podmac/", "/"), "555333/podmac") );

	REQUIRE(0 == bx::strCmp(bx::strTrim("abvgd", ""), "abvgd") );
	REQUIRE(0 == bx::strCmp(bx::strTrim(" \t a b\tv g d \t ", " \t"), "a b\tv g d") );

	bx::FilePath uri("/555333/podmac/");
	REQUIRE(0 == bx::strCmp(bx::strTrim(uri.getPath(), "/"), "555333/podmac") );
}
