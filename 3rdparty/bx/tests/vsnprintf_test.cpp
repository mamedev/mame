/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/string.h>

TEST_CASE("vsnprintf NULL buffer", "No output buffer provided.")
{
	REQUIRE(4 == bx::snprintf(NULL, 0, "test") );

	REQUIRE(1 == bx::snprintf(NULL, 0, "%d", 1) );
}

TEST_CASE("vsnprintf truncated", "Truncated output buffer.")
{
	char buffer[7];

	REQUIRE(10 == bx::snprintf(buffer, BX_COUNTOF(buffer), "Ten chars!") );
	REQUIRE(0  == strcmp(buffer, "Ten ch") );
}

static bool test(const char* _expected, const char* _format, ...)
{
	int32_t max = (int32_t)strlen(_expected) + 1;
	char* temp = (char*)alloca(max);

	va_list argList;
	va_start(argList, _format);
	int32_t len = bx::vsnprintf(temp, max, _format, argList);
	va_end(argList);

	bool result = true
		&& len == max-1
		&& 0   == strcmp(_expected, temp)
		;

	if (!result)
	{
		printf("result (%d) %s, expected (%d) %s\n", len, temp, max-1, _expected);
	}

	return result;
}

TEST_CASE("vsnprintf f", "")
{
	REQUIRE(test("1.337", "%0.3f", 1.337) );
	REQUIRE(test("  13.370", "%8.3f", 13.37) );
	REQUIRE(test("  13.370", "%*.*f", 8, 3, 13.37) );
	REQUIRE(test("13.370  ", "%-8.3f", 13.37) );
	REQUIRE(test("13.370  ", "%*.*f", -8, 3, 13.37) );
}

TEST_CASE("vsnprintf d/u/x", "")
{
	REQUIRE(test("1337", "%d", 1337) );

	REQUIRE(test("1337", "%x", 0x1337) );
}

TEST_CASE("vsnprintf", "")
{
	REQUIRE(test("x", "%c", 'x') );

	REQUIRE(test("hello, world!", "%s, %s!", "hello", "world") );
}
