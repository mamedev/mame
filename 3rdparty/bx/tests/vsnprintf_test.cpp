/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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

	REQUIRE(test(" 1.0",     "%4.1f",    1.0) );
	REQUIRE(test(" 1.500",   "%6.3f",    1.5) );
	REQUIRE(test("0001.500", "%08.3f",   1.5) );
	REQUIRE(test("+001.500", "%+08.3f",  1.5) );
	REQUIRE(test("-001.500", "%+08.3f", -1.5) );
	REQUIRE(test("0.0039",   "%.4f",     0.00390625) );

	REQUIRE(test("0.003906",     "%f",   0.00390625) );
	REQUIRE(test("-1.234567e-9", "%f",  -1.234567e-9) );

	REQUIRE(test("-1e-9",            "%.0f",  -1.234567e-9) );
	REQUIRE(test("-1.2e-9",          "%.1f",  -1.234567e-9) );
	REQUIRE(test("-1.23e-9",         "%.2f",  -1.234567e-9) );
	REQUIRE(test("-1.234e-9",        "%.3f",  -1.234567e-9) );
	REQUIRE(test("-1.2345e-9",       "%.4f",  -1.234567e-9) );
	REQUIRE(test("-1.23456e-9",      "%.5f",  -1.234567e-9) );
	REQUIRE(test("-1.234567e-9",     "%.6f",  -1.234567e-9) );
	REQUIRE(test("-1.2345670e-9",    "%.7f",  -1.234567e-9) );
	REQUIRE(test("-1.23456700e-9",   "%.8f",  -1.234567e-9) );
	REQUIRE(test("-1.234567000e-9",  "%.9f",  -1.234567e-9) );
	REQUIRE(test("-1.2345670000e-9", "%.10f", -1.234567e-9) );

	REQUIRE(test("3.141592",           "%f",    3.1415926535897932) );
	REQUIRE(test("3.141592",           "%F",    3.1415926535897932) );
	REQUIRE(test("3",                  "%.0f",  3.1415926535897932) );
	REQUIRE(test("3.1",                "%.1f",  3.1415926535897932) );
	REQUIRE(test("3.14",               "%.2f",  3.1415926535897932) );
	REQUIRE(test("3.141",              "%.3f",  3.1415926535897932) );
	REQUIRE(test("3.1415",             "%.4f",  3.1415926535897932) );
	REQUIRE(test("3.14159",            "%.5f",  3.1415926535897932) );
	REQUIRE(test("3.141592",           "%.6f",  3.1415926535897932) );
	REQUIRE(test("3.1415926",          "%.7f",  3.1415926535897932) );
	REQUIRE(test("3.14159265",         "%.8f",  3.1415926535897932) );
	REQUIRE(test("3.141592653",        "%.9f",  3.1415926535897932) );
	REQUIRE(test("3.1415926535",       "%.10f", 3.1415926535897932) );
	REQUIRE(test("3.14159265358",      "%.11f", 3.1415926535897932) );
	REQUIRE(test("3.141592653589",     "%.12f", 3.1415926535897932) );
	REQUIRE(test("3.1415926535897",    "%.13f", 3.1415926535897932) );
	REQUIRE(test("3.14159265358979",   "%.14f", 3.1415926535897932) );
	REQUIRE(test("3.141592653589793",  "%.15f", 3.1415926535897932) );
	REQUIRE(test("3.1415926535897930", "%.16f", 3.1415926535897932) );
	REQUIRE(test("3.1415926535897930", "%.16F", 3.1415926535897932) );

	REQUIRE(test("-3.141592e-9",           "%f",    -3.1415926535897932e-9) );
	REQUIRE(test("-3.141592E-9",           "%F",    -3.1415926535897932e-9) );
	REQUIRE(test("-3e-9",                  "%.0f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.1e-9",                "%.1f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.14e-9",               "%.2f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.141e-9",              "%.3f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415e-9",             "%.4f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.14159e-9",            "%.5f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.141592e-9",           "%.6f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415926e-9",          "%.7f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.14159265e-9",         "%.8f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.141592653e-9",        "%.9f",  -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415926535e-9",       "%.10f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.14159265358e-9",      "%.11f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.141592653589e-9",     "%.12f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415926535897e-9",    "%.13f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.14159265358979e-9",   "%.14f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.141592653589793e-9",  "%.15f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415926535897930e-9", "%.16f", -3.1415926535897932e-9) );
	REQUIRE(test("-3.1415926535897930E-9", "%.16F", -3.1415926535897932e-9) );

	REQUIRE(test("1e-12", "%f", 1e-12));

	REQUIRE(test("0.00390625",          "%.8f",  0.00390625) );
	REQUIRE(test("-0.00390625",         "%.8f", -0.00390625) );
	REQUIRE(test("1.50000000000000000", "%.17f", 1.5) );
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
	REQUIRE(test("                2471", "%20o",    1337) );
	REQUIRE(test("00000000000000002471", "%020o",   1337) );

	REQUIRE(test("1337", "%u", 1337) );
	REQUIRE(test("1337                ", "%-20u",  1337) );
	REQUIRE(test("4294965959          ", "%-20u", -1337) );

	REQUIRE(test("1337", "%x", 0x1337) );
	REQUIRE(test("1234abcd            ", "%-20x",  0x1234abcd) );
	REQUIRE(test("1234ABCD            ", "%-20X",  0x1234abcd) );
	REQUIRE(test("edcb5433            ", "%-20x", -0x1234abcd) );
	REQUIRE(test("EDCB5433            ", "%-20X", -0x1234abcd) );
	REQUIRE(test("            1234abcd", "% 20x",  0x1234abcd) );
	REQUIRE(test("            1234ABCD", "% 20X",  0x1234abcd) );
	REQUIRE(test("            edcb5433", "% 20x", -0x1234abcd) );
	REQUIRE(test("            EDCB5433", "% 20X", -0x1234abcd) );
	REQUIRE(test("0000000000001234abcd", "%020x",  0x1234abcd) );
	REQUIRE(test("0000000000001234ABCD", "%020X",  0x1234abcd) );
	REQUIRE(test("000000000000edcb5433", "%020x", -0x1234abcd) );
	REQUIRE(test("000000000000EDCB5433", "%020X", -0x1234abcd) );

	REQUIRE(test("0xf",        "0x%01x", -1) );
	REQUIRE(test("0xff",       "0x%02x", -1) );
	REQUIRE(test("0xfff",      "0x%03x", -1) );
	REQUIRE(test("0xffff",     "0x%04x", -1) );
	REQUIRE(test("0xfffff",    "0x%05x", -1) );
	REQUIRE(test("0xffffff",   "0x%06x", -1) );
	REQUIRE(test("0xfffffff",  "0x%07x", -1) );
	REQUIRE(test("0xffffffff", "0x%08x", -1) );

	REQUIRE(test("  -1", "% 4i", -1) );
	REQUIRE(test("  -1", "% 4i", -1) );
	REQUIRE(test("   0", "% 4i",  0) );
	REQUIRE(test("   1", "% 4i",  1) );
	REQUIRE(test("   1", "% 4o",  1) );
	REQUIRE(test("  +1", "%+4i",  1) );
	REQUIRE(test("  +1", "%+4o",  1) );
	REQUIRE(test("  +0", "%+4i",  0) );
	REQUIRE(test("  -1", "%+4i", -1) );
	REQUIRE(test("0001", "%04i",  1) );
	REQUIRE(test("0001", "%04o",  1) );
	REQUIRE(test("0000", "%04i",  0) );
	REQUIRE(test("0000", "%04o",  0) );
	REQUIRE(test("-001", "%04i", -1) );
	REQUIRE(test("+001", "%+04i", 1) );

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

TEST_CASE("vsnprintf t")
{
	size_t size = size_t(-1);

	REQUIRE(test("-1", "%td", size) );

	REQUIRE(test("3221225472", "%td", size_t(3221225472) ) );
}

TEST_CASE("vsnprintf n")
{
	char temp[64];

	int32_t p0, p1, p2;
	bx::snprintf(temp, sizeof(temp), "%n", &p0);
	REQUIRE(0 == p0);

	bx::snprintf(temp, sizeof(temp), "01%n23%n45%n", &p0, &p1, &p2);
	REQUIRE(2 == p0);
	REQUIRE(4 == p1);
	REQUIRE(6 == p2);
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

	REQUIRE(test("h",     "%1s", "hello") );
	REQUIRE(test("he",    "%2s", "hello") );
	REQUIRE(test("hel",   "%3s", "hello") );
	REQUIRE(test("hell",  "%4s", "hello") );
	REQUIRE(test("hello", "%5s", "hello") );

	bx::StringView str("0hello1world2");
	bx::StringView hello(str, 1, 5);
	bx::StringView world(str, 7, 5);
	REQUIRE(test("hello, world!", "%.*s, %.*s!"
		, hello.getLength(), hello.getPtr()
		, world.getLength(), world.getPtr()
		) );

	REQUIRE(test("hello, world!", "%S, %S!"
		, &hello
		, &world
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

TEST_CASE("snprintf invalid")
{
	char temp[64];
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-0", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-03", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-03.", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-03.0", 1) );
	REQUIRE(0 == bx::snprintf(temp, sizeof(temp), "%-03.0t", 1) );
}
