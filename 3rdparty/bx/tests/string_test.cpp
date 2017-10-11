/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/string.h>
#include <bx/crtimpl.h>
#include <bx/handlealloc.h>

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

TEST_CASE("strnlen", "")
{
	const char* test = "test";

	REQUIRE(0 == bx::strnlen(test, 0) );
	REQUIRE(2 == bx::strnlen(test, 2) );
	REQUIRE(4 == bx::strnlen(test, INT32_MAX) );
}

TEST_CASE("strlncpy", "")
{
	char dst[128];
	size_t num;

	num = bx::strlncpy(dst, 1, "blah");
	REQUIRE(num == 0);

	num = bx::strlncpy(dst, 3, "blah", 3);
	REQUIRE(0 == bx::strncmp(dst, "bl") );
	REQUIRE(num == 2);

	num = bx::strlncpy(dst, sizeof(dst), "blah", 3);
	REQUIRE(0 == bx::strncmp(dst, "bla") );
	REQUIRE(num == 3);

	num = bx::strlncpy(dst, sizeof(dst), "blah");
	REQUIRE(0 == bx::strncmp(dst, "blah") );
	REQUIRE(num == 4);
}

TEST_CASE("strlncat", "")
{
	char dst[128] = { '\0' };

	REQUIRE(0 == bx::strlncat(dst, 1, "cat") );

	REQUIRE(4 == bx::strlncpy(dst, 5, "copy") );
	REQUIRE(3 == bx::strlncat(dst, 8, "cat") );
	REQUIRE(0 == bx::strncmp(dst, "copycat") );

	REQUIRE(1 == bx::strlncat(dst, BX_COUNTOF(dst), "------", 1) );
	REQUIRE(3 == bx::strlncat(dst, BX_COUNTOF(dst), "cat") );
	REQUIRE(0 == bx::strncmp(dst, "copycat-cat") );
}

TEST_CASE("strincmp", "")
{
	REQUIRE(0 == bx::strincmp("test", "test") );
	REQUIRE(0 == bx::strincmp("test", "testestes", 4) );
	REQUIRE(0 == bx::strincmp("testestes", "test", 4) );
	REQUIRE(0 != bx::strincmp("preprocess", "platform") );

	const char* abvgd = "abvgd";
	const char* abvgx = "abvgx";
	const char* empty = "";
	REQUIRE(0 == bx::strincmp(abvgd, abvgd) );
	REQUIRE(0 == bx::strincmp(abvgd, abvgx, 4) );

	REQUIRE(0 >  bx::strincmp(abvgd, abvgx) );
	REQUIRE(0 >  bx::strincmp(empty, abvgd) );

	REQUIRE(0 <  bx::strincmp(abvgx, abvgd) );
	REQUIRE(0 <  bx::strincmp(abvgd, empty) );
}

TEST_CASE("strnchr", "")
{
	const char* test = "test";
	REQUIRE(NULL == bx::strnchr(test, 's', 0) );
	REQUIRE(NULL == bx::strnchr(test, 's', 2) );
	REQUIRE(&test[2] == bx::strnchr(test, 's') );
}

TEST_CASE("strnrchr", "")
{
	const char* test = "test";
	REQUIRE(NULL == bx::strnrchr(test, 's', 0) );
	REQUIRE(NULL == bx::strnrchr(test, 's', 1) );
	REQUIRE(&test[2] == bx::strnrchr(test, 's') );
}

TEST_CASE("stristr", "")
{
	const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

	REQUIRE(NULL == bx::stristr(test, "quick", 8) );
	REQUIRE(NULL == bx::stristr(test, "quick1") );
	REQUIRE(&test[4] == bx::stristr(test, "quick", 9) );
	REQUIRE(&test[4] == bx::stristr(test, "quick") );
}

TEST_CASE("strnstr", "")
{
	const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

	REQUIRE(NULL == bx::strnstr(test, "quick", 8) );
	REQUIRE(NULL == bx::strnstr(test, "quick1") );
	REQUIRE(NULL == bx::strnstr(test, "quick", 9) );
	REQUIRE(NULL == bx::strnstr(test, "quick") );

	REQUIRE(NULL == bx::strnstr(test, "Quick", 8) );
	REQUIRE(NULL == bx::strnstr(test, "Quick1") );
	REQUIRE(&test[4] == bx::strnstr(test, "Quick", 9) );
	REQUIRE(&test[4] == bx::strnstr(test, "Quick") );
}

template<typename Ty>
static bool testToString(Ty _value, const char* _expected)
{
	char tmp[1024];
	int32_t num = bx::toString(tmp, BX_COUNTOF(tmp), _value);
	int32_t len = (int32_t)bx::strnlen(_expected);
	if (0 == bx::strncmp(tmp, _expected)
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
}

TEST_CASE("StringView", "")
{
	bx::StringView sv("test");
	REQUIRE(4 == sv.getLength() );

	bx::CrtAllocator crt;
	g_allocator = &crt;

	typedef bx::StringT<&g_allocator> String;

	String st(sv);
	REQUIRE(4 == st.getLength() );

	st.append("test");
	REQUIRE(8 == st.getLength() );

	st.append("test", 2);
	REQUIRE(10 == st.getLength() );

	REQUIRE(0 == bx::strncmp(st.getPtr(), "testtestte") );

	st.clear();
	REQUIRE(0 == st.getLength() );
	REQUIRE(4 == sv.getLength() );

	st.append("test");
	REQUIRE(4 == st.getLength() );

	sv.clear();
	REQUIRE(0 == sv.getLength() );
}
