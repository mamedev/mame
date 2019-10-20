/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/string.h>
#include <bx/readerwriter.h>

#include <limits>
#include <inttypes.h>

TEST_CASE("vsnprintf NULL buffer", "No output buffer provided.")
{
	REQUIRE(4 == bx::snprintf(NULL, 0, "test") );

	REQUIRE(1 == bx::snprintf(NULL, 0, "%d", 1) );
}

TEST_CASE("vsnprintf truncated", "Truncated output buffer.")
{
	char buffer5[5]; // fit
	REQUIRE(4 == bx::snprintf(buffer5, BX_COUNTOF(buffer5), "abvg") );
	REQUIRE(0 == bx::strCmp(buffer5, "abvg") );

	char buffer7[7]; // truncate
	REQUIRE(10 == bx::snprintf(buffer7, BX_COUNTOF(buffer7), "Ten chars!") );
	REQUIRE(0  == bx::strCmp(buffer7, "Ten ch") );
}

static bool test(const char* _expected, const char* _format, ...)
{
	int32_t max = (int32_t)bx::strLen(_expected) + 1;
	char* temp = (char*)alloca(max);

	va_list argList;
	va_start(argList, _format);
	int32_t len = bx::vsnprintf(temp, max, _format, argList);
	va_end(argList);

	bool result = true
		&& len == max-1
		&& 0   == bx::strCmp(_expected, temp)
		;

	if (!result)
	{
		printf("result (%d) '%s', expected (%d) '%s'\n", len, temp, max-1, _expected);
	}

	return result;
}

TEST_CASE("vsnprintf f")
{
	REQUIRE(test("1.337",    "%0.3f", 1.337) );
	REQUIRE(test("  13.370", "%8.3f", 13.37) );
	REQUIRE(test("  13.370", "%*.*f", 8, 3, 13.37) );
	REQUIRE(test("13.370  ", "%-8.3f", 13.37) );
	REQUIRE(test("13.370  ", "%*.*f", -8, 3, 13.37) );

	REQUIRE(test("nan     ", "%-8f",  std::numeric_limits<double>::quiet_NaN() ) );
	REQUIRE(test("     nan", "%8f",   std::numeric_limits<double>::quiet_NaN() ) );
	REQUIRE(test("-NAN    ", "%-8F", -std::numeric_limits<double>::quiet_NaN() ) );

	REQUIRE(test("     inf", "%8f",   std::numeric_limits<double>::infinity() ) );
	REQUIRE(test("inf     ", "%-8f",  std::numeric_limits<double>::infinity() ) );
	REQUIRE(test("    -INF", "%8F",  -std::numeric_limits<double>::infinity() ) );
}

TEST_CASE("vsnprintf d/i/o/u/x")
{
	REQUIRE(test("1337", "%d", 1337) );
	REQUIRE(test("1337                ", "%-20d",  1337) );
	REQUIRE(test("-1337               ", "%-20d", -1337) );

	REQUIRE(test("1337", "%i", 1337) );
	REQUIRE(test("1337                ", "%-20i",  1337) );
	REQUIRE(test("-1337               ", "%-20i", -1337) );

	REQUIRE(test("1337", "%o", 01337) );
	REQUIRE(test("2471", "%o", 1337) );
	REQUIRE(test("1337                ", "%-20o",  01337) );
	REQUIRE(test("37777776441         ", "%-20o", -01337) );

	REQUIRE(test("1337", "%u", 1337) );
	REQUIRE(test("1337                ", "%-20u",  1337) );
	REQUIRE(test("4294965959          ", "%-20u", -1337) );

	REQUIRE(test("1337", "%x", 0x1337) );
	REQUIRE(test("1234abcd            ", "%-20x",  0x1234abcd) );
	REQUIRE(test("1234ABCD            ", "%-20X",  0x1234abcd) );
	REQUIRE(test("edcb5433            ", "%-20x", -0x1234abcd) );
	REQUIRE(test("EDCB5433            ", "%-20X", -0x1234abcd) );
	REQUIRE(test("0000000000001234abcd", "%020x",  0x1234abcd) );
	REQUIRE(test("0000000000001234ABCD", "%020X",  0x1234abcd) );
	REQUIRE(test("000000000000edcb5433", "%020x", -0x1234abcd) );
	REQUIRE(test("000000000000EDCB5433", "%020X", -0x1234abcd) );

	if (BX_ENABLED(BX_ARCH_32BIT) )
	{
		REQUIRE(test("2147483647", "%jd", INTMAX_MAX) );
	}
	else
	{
		REQUIRE(test("9223372036854775807", "%jd", INTMAX_MAX) );
	}

	REQUIRE(test("18446744073709551615", "%" PRIu64, UINT64_MAX) );
	REQUIRE(test("ffffffffffffffff", "%016" PRIx64, UINT64_MAX) );
}

TEST_CASE("vsnprintf modifiers")
{
	REQUIRE(test("|  1.000000|", "|%10f|",      1.0f) );
	REQUIRE(test("|1.000000  |", "|%-10f|",     1.0f) );
	REQUIRE(test("|001.000000|", "|%010f|",     1.0f) );
	REQUIRE(test("|0000000001|", "|%010.0f|",   1.0f) );
	REQUIRE(test("|000000001.|", "|%#010.0f|",  1.0f) );
	REQUIRE(test("|         1|", "|%10.0f|",    1.0f) );
	REQUIRE(test("|        1.|", "|%#10.0f|",   1.0f) );
	REQUIRE(test("|       +1.|", "|%#+10.0f|",  1.0f) );
	REQUIRE(test("|1         |", "|%-10.0f|",   1.0f) );
	REQUIRE(test("|1.        |", "|%#-10.0f|",  1.0f) );
	REQUIRE(test("|+1.       |", "|%+#-10.0f|", 1.0f) );
}

TEST_CASE("vsnprintf p")
{
	REQUIRE(test("0xbadc0de", "%p", (void*)0xbadc0de) );
	REQUIRE(test("0xbadc0de           ", "%-20p", (void*)0xbadc0de) );
}

TEST_CASE("vsnprintf s")
{
	REQUIRE(test("(null)", "%s", NULL) );
}

TEST_CASE("vsnprintf g")
{
	REQUIRE(test("   0.01",  "%7.2g", .01) );
	REQUIRE(test(" 0.0123",  "%7.4G", .0123) );
//	REQUIRE(test("1.23e+05", "%.3g",  123000.25) );
//	REQUIRE(test("1e+05",    "%.0g",  123000.25) );
}

TEST_CASE("vsnprintf")
{
	REQUIRE(test("x", "%c", 'x') );
	REQUIRE(test("x                   ", "%-20c", 'x') );

	REQUIRE(test("hello               ", "%-20s", "hello") );
	REQUIRE(test("hello, world!", "%s, %s!", "hello", "world") );

	bx::StringView str("0hello1world2");
	bx::StringView hello(str, 1, 5);
	bx::StringView world(str, 7, 5);
	REQUIRE(test("hello, world!", "%.*s, %.*s!"
		, hello.getLength(), hello.getPtr()
		, world.getLength(), world.getPtr()
		) );
}

TEST_CASE("vsnprintf write")
{
	char tmp[64];
	bx::StaticMemoryBlock mb(tmp, sizeof(tmp));
	bx::MemoryWriter writer(&mb);

	bx::Error err;
	int32_t len = bx::write(&writer, &err, "%d", 1389);
	REQUIRE(err.isOk());
	REQUIRE(len == 4);

	bx::StringView str(tmp, len);
	REQUIRE(0 == bx::strCmp(str, "1389") );
}
