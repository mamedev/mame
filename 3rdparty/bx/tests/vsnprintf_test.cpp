/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/string.h>
#include <bx/readerwriter.h>

#include <limits>
#include <inttypes.h>

TEST_CASE("No output buffer provided.", "[string][printf]")
{
	REQUIRE(4 == bx::snprintf(NULL, 0, "test") );

	REQUIRE(1 == bx::snprintf(NULL, 0, "%d", 1) );
}

TEST_CASE("Truncated output buffer.", "[string][printf]")
{
	REQUIRE(4 == bx::snprintf(NULL, 0, "abvg") );

	char buffer15[15]; // fit
	REQUIRE(4    == bx::snprintf(buffer15, BX_COUNTOF(buffer15), "abvg") );
	REQUIRE('\0' == buffer15[4]);
	REQUIRE(0    == bx::strCmp(buffer15, "abvg") );

	char buffer1[1]; // truncate
	REQUIRE(4    == bx::snprintf(buffer1, BX_COUNTOF(buffer1), "abvg") );
	REQUIRE('\0' == buffer1[BX_COUNTOF(buffer1)-1]);

	char buffer7[7]; // truncate
	REQUIRE(10   == bx::snprintf(NULL, 0, "Ten chars!") );
	REQUIRE(10   == bx::snprintf(buffer7, BX_COUNTOF(buffer7), "Ten chars!") );
	REQUIRE('\0' == buffer7[BX_COUNTOF(buffer7)-1]);
	REQUIRE(0    == bx::strCmp(buffer7, "Ten ch") );

	REQUIRE(7    == bx::snprintf(NULL, 0, "Seven67") );
	REQUIRE(7    == bx::snprintf(buffer7, BX_COUNTOF(buffer7), "Seven67") );
	REQUIRE('\0' == buffer7[BX_COUNTOF(buffer7)-1]);
	REQUIRE(0    == bx::strCmp(buffer7, "Seven6") );

	REQUIRE(11   == bx::snprintf(NULL, 0, "SevenEleven") );
	REQUIRE(11   == bx::snprintf(buffer7, BX_COUNTOF(buffer7), "SevenEleven") );
	REQUIRE('\0' == buffer7[BX_COUNTOF(buffer7)-1]);
	REQUIRE(0    == bx::strCmp(buffer7, "SevenE") );
}

template<bool StdCompliantT>
static bool test(const char* _expected, const char* _format, va_list _argList)
{
	int32_t max = (int32_t)bx::strLen(_expected) + 1;
	char* bxTemp = (char*)alloca(max);

	va_list argList;
	va_copy(argList, _argList);
	const int32_t bxLen = bx::vsnprintf(bxTemp, max, _format, argList);

	bool result = true
		&& bxLen == max-1
		&&     0 == bx::strCmp(_expected, bxTemp)
		;

	char*  crtTemp = NULL;
	int32_t crtLen = 0;

	if (!result
	||  StdCompliantT)
	{
		BX_ASSERT(bx::strFind(_format, "%S").isEmpty()
			, "String format test is using '%%S' bx::StringView specific format specifier which is not standard compliant. "
			  "Use `testNotStdCompliant` string testing method."
			);

		crtTemp = (char*)alloca(max);

		va_copy(argList, _argList);
		crtLen = ::vsnprintf(crtTemp, max, _format, argList);

		result &= true
			&& crtLen == bxLen
			&&      0 == bx::strCmp(bx::StringView(bxTemp, bxLen), bx::StringView(crtTemp, crtLen) )
			;
	}

	if (!result)
	{
		printf("---\n");
		printf("printf format '%s'\n", _format);
		printf("       result (%4d) '%s'\n", bxLen, bxTemp);
		printf("     expected (%4d) '%s'\n", max-1, _expected);
		printf("CRT vsnprintf (%4d) '%s'\n", crtLen, crtTemp);
	}

	return result;
}

// Test against CRT's vsnprintf implementation.
static bool test(const char* _expected, const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	const bool result = test<false>(_expected, _format, argList);
	va_end(argList);

	return result;
}

// Skip test against CRT's vsnprintf implementation.
static bool testNotStdCompliant(const char* _expected, const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	const bool result = test<false>(_expected, _format, argList);
	va_end(argList);

	return result;
}

TEST_CASE("Format %f", "[string][printf]")
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
	REQUIRE(testNotStdCompliant("-1.234567e-9", "%f",  -1.234567e-9) );

	REQUIRE(testNotStdCompliant("-1e-9",            "%.0f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.2e-9",          "%.1f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.23e-9",         "%.2f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.234e-9",        "%.3f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.2345e-9",       "%.4f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.23456e-9",      "%.5f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.234567e-9",     "%.6f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.2345670e-9",    "%.7f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.23456700e-9",   "%.8f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.234567000e-9",  "%.9f",  -1.234567e-9) );
	REQUIRE(testNotStdCompliant("-1.2345670000e-9", "%.10f", -1.234567e-9) );

	REQUIRE(testNotStdCompliant("3.141592",           "%f",    3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141592",           "%F",    3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3",                  "%.0f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1",                "%.1f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.14",               "%.2f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141",              "%.3f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415",             "%.4f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.14159",            "%.5f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141592",           "%.6f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415926",          "%.7f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.14159265",         "%.8f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141592653",        "%.9f",  3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415926535",       "%.10f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.14159265358",      "%.11f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141592653589",     "%.12f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415926535897",    "%.13f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.14159265358979",   "%.14f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.141592653589793",  "%.15f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415926535897930", "%.16f", 3.1415926535897932) );
	REQUIRE(testNotStdCompliant("3.1415926535897930", "%.16F", 3.1415926535897932) );

	REQUIRE(testNotStdCompliant("-3.141592e-9",           "%f",    -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141592E-9",           "%F",    -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3e-9",                  "%.0f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1e-9",                "%.1f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.14e-9",               "%.2f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141e-9",              "%.3f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415e-9",             "%.4f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.14159e-9",            "%.5f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141592e-9",           "%.6f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415926e-9",          "%.7f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.14159265e-9",         "%.8f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141592653e-9",        "%.9f",  -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415926535e-9",       "%.10f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.14159265358e-9",      "%.11f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141592653589e-9",     "%.12f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415926535897e-9",    "%.13f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.14159265358979e-9",   "%.14f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.141592653589793e-9",  "%.15f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415926535897930e-9", "%.16f", -3.1415926535897932e-9) );
	REQUIRE(testNotStdCompliant("-3.1415926535897930E-9", "%.16F", -3.1415926535897932e-9) );

	REQUIRE(testNotStdCompliant("1e-12", "%f", 1e-12));

	REQUIRE(test("0.00390625",          "%.8f",  0.00390625) );
	REQUIRE(test("-0.00390625",         "%.8f", -0.00390625) );
	REQUIRE(test("1.50000000000000000", "%.17f", 1.5) );
}

TEST_CASE("Format %d, %i, %o, %u, %x", "[string][printf]")
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

	REQUIRE(testNotStdCompliant("0xf",        "0x%01x", -1) );
	REQUIRE(testNotStdCompliant("0xff",       "0x%02x", -1) );
	REQUIRE(testNotStdCompliant("0xfff",      "0x%03x", -1) );
	REQUIRE(testNotStdCompliant("0xffff",     "0x%04x", -1) );
	REQUIRE(testNotStdCompliant("0xfffff",    "0x%05x", -1) );
	REQUIRE(testNotStdCompliant("0xffffff",   "0x%06x", -1) );
	REQUIRE(testNotStdCompliant("0xfffffff",  "0x%07x", -1) );
	REQUIRE(testNotStdCompliant("0xffffffff", "0x%08x", -1) );

	REQUIRE(test("  -1", "% 4i", -1) );
	REQUIRE(test("  -1", "% 4i", -1) );
	REQUIRE(test("   0", "% 4i",  0) );
	REQUIRE(test("   1", "% 4i",  1) );
	REQUIRE(test("   1", "% 4o",  1) );
	REQUIRE(test("  +1", "%+4i",  1) );
	REQUIRE(testNotStdCompliant("  +1", "%+4o",  1) );
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

TEST_CASE("Format modifiers", "[string][printf]")
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

	REQUIRE(test("|     00013:    -00089|", "|%10.5d:%10.5d|",   13, -89) );
	REQUIRE(test("|    -00013:    +00089|", "|%10.5d:%+10.5d|", -13,  89) );
	REQUIRE(test("|    -00013:    -00089|", "|%10.5d:%10.5d|",  -13, -89) );
}

TEST_CASE("Format %p", "[string][printf]")
{
	REQUIRE(test("0xbadc0de", "%p", (void*)0xbadc0de) );
	REQUIRE(test("0xbadc0de           ", "%-20p", (void*)0xbadc0de) );
}

TEST_CASE("Format %s", "[string][printf]")
{
	REQUIRE(test("(null)", "%s", NULL) );
}

TEST_CASE("Format %td", "[string][printf]")
{
	size_t size = size_t(-1);

	REQUIRE(test("-1", "%td", size) );

	REQUIRE(test("3221225472", "%td", size_t(3221225472) ) );
}

TEST_CASE("Format %n", "[string][printf]")
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

TEST_CASE("Format %g", "[string][printf]")
{
	REQUIRE(test("   0.01",  "%7.2g", .01) );
	REQUIRE(test(" 0.0123",  "%7.4G", .0123) );
//	REQUIRE(test("1.23e+05", "%.3g",  123000.25) );
//	REQUIRE(test("1e+05",    "%.0g",  123000.25) );
}

TEST_CASE("Format %c, %s, %S", "[string][printf]")
{
	REQUIRE(test("x", "%c", 'x') );
	REQUIRE(test("x                   ", "%-20c", 'x') );

	REQUIRE(test("hello               ", "%-20s", "hello") );
	REQUIRE(test("hello, world!", "%s, %s!", "hello", "world") );

	REQUIRE(testNotStdCompliant("h",     "%1s", "hello") );
	REQUIRE(testNotStdCompliant("he",    "%2s", "hello") );
	REQUIRE(testNotStdCompliant("hel",   "%3s", "hello") );
	REQUIRE(testNotStdCompliant("hell",  "%4s", "hello") );
	REQUIRE(testNotStdCompliant("hello", "%5s", "hello") );

	bx::StringView str("0hello1world2");
	bx::StringView hello(str, 1, 5);
	bx::StringView world(str, 7, 5);
	REQUIRE(test("hello, world!", "%.*s, %.*s!"
		, hello.getLength(), hello.getPtr()
		, world.getLength(), world.getPtr()
		) );

	REQUIRE(testNotStdCompliant("hello, world!", "%S, %S!"
		, &hello
		, &world
		) );
}

TEST_CASE("WriterI", "[string][printf]")
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

TEST_CASE("Invalid", "[string][printf]")
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
