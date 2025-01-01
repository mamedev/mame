/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/filepath.h>
#include <bx/string.h>
#include <bx/handlealloc.h>
#include <bx/sort.h>
#include <string>

bx::AllocatorI* g_allocator;

TEST_CASE("StringLiteral", "[string]")
{
	constexpr bx::StringLiteral tmp[] = { "1389", "abvgd", "mac", "pod" };

	REQUIRE(bx::isSorted(tmp, BX_COUNTOF(tmp) ) );

	STATIC_REQUIRE(4 == tmp[0].getLength() );
	REQUIRE(4 == bx::strLen(tmp[0]) );
	REQUIRE(0 == bx::strCmp("1389", tmp[0]) );

	STATIC_REQUIRE(5 == tmp[1].getLength() );
	REQUIRE(5 == bx::strLen(tmp[1]) );
	REQUIRE(0 == bx::strCmp("abvgd", tmp[1]) );

	STATIC_REQUIRE(3 == tmp[2].getLength() );
	REQUIRE(3 == bx::strLen(tmp[2]) );
	REQUIRE(0 == bx::strCmp("mac", tmp[2]) );

	STATIC_REQUIRE(3 == tmp[3].getLength() );
	REQUIRE(3 == bx::strLen(tmp[3]) );
	REQUIRE(0 == bx::strCmp("pod", tmp[3]) );

	constexpr bx::StringLiteral copy(tmp[0]);

	STATIC_REQUIRE(4 == copy.getLength() );
	REQUIRE(4 == bx::strLen(copy) );
	REQUIRE(0 == bx::strCmp("1389", copy) );

	constexpr bx::StringView sv(tmp[1]);

	REQUIRE(5 == sv.getLength() );
	REQUIRE(5 == bx::strLen(sv) );
	REQUIRE(0 == bx::strCmp("abvgd", sv) );
}

TEST_CASE("stringPrintfTy", "[string]")
{
	std::string test;
	bx::stringPrintf(test, "printf into std::string.");
	REQUIRE(0 == bx::strCmp(bx::StringView(test.data(), int32_t(test.length() ) ), "printf into std::string.") );
}

TEST_CASE("prettify", "[string]")
{
	char tmp[1024];
	prettify(tmp, BX_COUNTOF(tmp), 4000, bx::Units::Kilo);
	REQUIRE(0 == bx::strCmp(tmp, "4.00 kB") );

	prettify(tmp, BX_COUNTOF(tmp), 4096, bx::Units::Kibi);
	REQUIRE(0 == bx::strCmp(tmp, "4.00 KiB") );
}

TEST_CASE("chars", "[string]")
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

TEST_CASE("strLen", "[string]")
{
	const char* test = "test";

	REQUIRE(0 == bx::strLen(test, 0) );
	REQUIRE(2 == bx::strLen(test, 2) );
	REQUIRE(4 == bx::strLen(test, INT32_MAX) );
}

TEST_CASE("strCopy", "[string]")
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

TEST_CASE("strCat", "[string]")
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

TEST_CASE("strCmp", "[string]")
{
	REQUIRE(0  < bx::strCmp("abvgd", "abv") );
	REQUIRE(0  < bx::strCmp("abvgd", "") );
	REQUIRE(0  > bx::strCmp("", "abvgd") );
	REQUIRE(0 != bx::strCmp(".tar.gz", ".") );
	REQUIRE(0 != bx::strCmp("meh", "meh/") );
}

TEST_CASE("strCmpI", "[string]")
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

TEST_CASE("strCmpV", "[string]")
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

TEST_CASE("strCmpV sort", "[string][sort]")
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

TEST_CASE("strRFind", "[string]")
{
	const char* test = "test";
	REQUIRE(bx::strRFind(bx::StringView(test, 0), 's').isEmpty() );
	REQUIRE(bx::strRFind(bx::StringView(test, 1), 's').isEmpty() );
	REQUIRE(&test[2] == bx::strRFind(test, 's').getPtr() );
	REQUIRE(&test[3] == bx::strRFind(test, 't').getPtr() );
}

TEST_CASE("strFindI", "[string]")
{
	const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

	REQUIRE(bx::strFindI(bx::StringView(test, 8), "quick").isEmpty() );
	REQUIRE(bx::strFindI(test, "quick1").isEmpty() );
	REQUIRE(&test[4] == bx::strFindI(bx::StringView(test, 9), "quick").getPtr() );
	REQUIRE(&test[4] == bx::strFindI(test, "quick").getPtr() );
}

TEST_CASE("strFind", "[string]")
{
	{
		const char* test = "test";

		REQUIRE(bx::strFind(bx::StringView(test, 0), 's').isEmpty() );
		REQUIRE(bx::strFind(bx::StringView(test, 2), 's').isEmpty() );
		REQUIRE(&test[2] == bx::strFind(test, 's').getPtr() );
	}

	{
		const char* test = "The Quick Brown Fox Jumps Over The Lazy Dog.";

		REQUIRE(bx::strFind(bx::StringView(test, 8), "quick").isEmpty() );
		REQUIRE(bx::strFind(test, "quick1").isEmpty() );
		REQUIRE(bx::strFind(bx::StringView(test, 9), "quick").isEmpty() );
		REQUIRE(bx::strFind(test, "quick").isEmpty() );

		REQUIRE(bx::strFind(bx::StringView(test, 8), "Quick").isEmpty() );
		REQUIRE(bx::strFind(test, "Quick1").isEmpty() );
		REQUIRE(&test[4] == bx::strFind(bx::StringView(test, 9), "Quick").getPtr() );
		REQUIRE(&test[4] == bx::strFind(test, "Quick").getPtr() );

		REQUIRE(bx::strFind("vgd", 'a').isEmpty() );
	}

	{
		bx::StringView test = bx::strFind("a", "a");
		REQUIRE(test.getLength() == 1);
		REQUIRE(*test.getPtr() == 'a');
	}

	{
		bx::StringView test = bx::strFind("a", bx::StringView("a ", 1) );
		REQUIRE(test.getLength() == 1);
		REQUIRE(*test.getPtr() == 'a');
	}
}

TEST_CASE("strSkip", "[string]")
{
	const bx::StringView t0("   test X");

	const bx::StringView t1 = bx::strLTrimSpace(t0);
	REQUIRE(0 == bx::strCmp(t1, "test", 4) );

	const bx::StringView t2 = bx::strLTrimNonSpace(t1);
	REQUIRE(0 == bx::strCmp(t2, " X", 2) );

	const bx::StringView t3("test");

	const bx::StringView t4 = bx::strLTrimNonSpace(t3);
	REQUIRE(t4.getTerm() == t4.getPtr() );
}

template<typename Ty>
static bool testToStringS(Ty _value, const char* _expected, char _separator = '\0')
{
	char tmp[1024];
	int32_t num = bx::toString(tmp, BX_COUNTOF(tmp), _value, 10, _separator);
	int32_t len = (int32_t)bx::strLen(_expected);
	if (0 == bx::strCmp(tmp, _expected)
	&&  num == len)
	{
		return true;
	}

	printf("result '%s' (%d), expected '%s' (%d)\n", tmp, num, _expected, len);
	return false;
}

TEST_CASE("toString intXX_t/uintXX_t", "[string]")
{
	REQUIRE(testToStringS(0,          "0") );
	REQUIRE(testToStringS(-256,       "-256") );
	REQUIRE(testToStringS(INT32_MAX,  "2147483647") );
	REQUIRE(testToStringS(UINT32_MAX, "4294967295") );
	REQUIRE(testToStringS(INT64_MAX,  "9223372036854775807") );
	REQUIRE(testToStringS(UINT64_MAX, "18446744073709551615") );

	REQUIRE(testToStringS(0,          "0", ',') );
	REQUIRE(testToStringS(-256,       "-256", ',') );
	REQUIRE(testToStringS(INT32_MAX,  "2,147,483,647", ',') );
	REQUIRE(testToStringS(UINT32_MAX, "4,294,967,295", ',') );
	REQUIRE(testToStringS(INT64_MAX,  "9,223,372,036,854,775,807", ',') );
	REQUIRE(testToStringS(UINT64_MAX, "18,446,744,073,709,551,615", ',') );
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

TEST_CASE("toString double", "[string]")
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
	REQUIRE(testToString(2.225073858507201e-308,  "2.225073858507201e-308") );
	REQUIRE(testToString(-79.39773355813419,      "-79.39773355813419") );
	REQUIRE(testToString(-1.234567e-9,            "-1.234567e-9") );
}

template<typename Ty>
static bool testFromString(Ty _value, const char* _input)
{
	char tmp[1024];
	bx::toString(tmp, BX_COUNTOF(tmp), _value);

	Ty lhs;
	bx::fromString(&lhs, tmp);

	Ty rhs;
	bx::fromString(&rhs, _input);

	if (lhs == rhs)
	{
		return true;
	}

	printf("result '%f', input '%s'\n", _value, _input);
	return false;
}

TEST_CASE("fromString float", "[string]")
{
	REQUIRE(testFromString<float>(std::numeric_limits<float>::min(),    "1.175494351e-38") );
	REQUIRE(testFromString<float>(std::numeric_limits<float>::lowest(), "-3.402823466e+38") );
	REQUIRE(testFromString<float>(std::numeric_limits<float>::max(),    "3.402823466e+38") );
}

TEST_CASE("fromString double", "[string]")
{
	REQUIRE(testFromString<double>(0.0,                     "0.0") );
	REQUIRE(testFromString<double>(-0.0,                    "-0.0") );
	REQUIRE(testFromString<double>(1.0,                     "1.0") );
	REQUIRE(testFromString<double>(-1.0,                    "-1.0") );
	REQUIRE(testFromString<double>(1.2345,                  "1.2345") );
	REQUIRE(testFromString<double>(1.2345678,               "1.2345678") );
	REQUIRE(testFromString<double>(0.123456789012,          "0.123456789012") );
	REQUIRE(testFromString<double>(123456.789,              "123456.789") );
	REQUIRE(testFromString<double>(1234567.8,               "1234567.8") );
	REQUIRE(testFromString<double>(-79.39773355813419,      "-79.39773355813419") );
	REQUIRE(testFromString<double>(0.000001,                "0.000001") );
	REQUIRE(testFromString<double>(0.0000001,               "1e-7") );
	REQUIRE(testFromString<double>(1e30,                    "1e30") );
	REQUIRE(testFromString<double>(1.234567890123456e30,    "1.234567890123456e30") );
	REQUIRE(testFromString<double>(-5e-324,                 "-5e-324") );
	REQUIRE(testFromString<double>(2.225073858507201e-308,  "2.225073858507201e-308") );
	REQUIRE(testFromString<double>(2.2250738585072014e-308, "2.2250738585072014e-308") );
	REQUIRE(testFromString<double>(1.7976931348623157e308,  "1.7976931348623157e308") );
	REQUIRE(testFromString<double>(0.00000123123123,        "0.00000123123123") );
	REQUIRE(testFromString<double>(0.000000123123123,       "1.23123123e-7") );
	REQUIRE(testFromString<double>(123123.123,              "123123.123") );
	REQUIRE(testFromString<double>(1231231.23,              "1231231.23") );
	REQUIRE(testFromString<double>(0.000000000123123,       "1.23123e-10") );
	REQUIRE(testFromString<double>(0.0000000001,            "1e-10") );
	REQUIRE(testFromString<double>(-270.000000,             "-270.0") );
	REQUIRE(testFromString<double>(2.2250738585072011e-308, "2.2250738585072011e-308") ); // https://web.archive.org/web/20181112222123/https://www.exploringbinary.com/php-hangs-on-numeric-value-2-2250738585072011e-308/
	REQUIRE(testFromString<double>(2.2250738585072009e-308, "2.2250738585072009e-308") ); // Max subnormal double
	REQUIRE(testFromString<double>(4.9406564584124654e-324, "4.9406564584124654e-324") ); // Min denormal
	REQUIRE(testFromString<double>(1.7976931348623157e+308, "1.7976931348623157e+308") ); // Max double

//  warning: magnitude of floating-point constant too small for type 'double'; minimum is 4.9406564584124654E-324
//	REQUIRE(testFromString<double>(1e-10000,                "0.0") );                     // Must underflow
//	integer literal is too large to be represented in any integer type
//	REQUIRE(testFromString<double>(18446744073709551616,    "18446744073709551616.0") );  // 2^64 (max of uint64_t + 1, force to use double)
//	REQUIRE(testFromString<double>(-9223372036854775809,    "-9223372036854775809.0") );  // -2^63 - 1(min of int64_t + 1, force to use double)

	REQUIRE(testFromString<double>(0.9868011474609375,      "0.9868011474609375") );      // https://github.com/miloyip/rapidjson/issues/120
	REQUIRE(testFromString<double>(123e34,                  "123e34") );
	REQUIRE(testFromString<double>(45913141877270640000.0,  "45913141877270640000.0") );
	REQUIRE(testFromString<double>(std::numeric_limits<double>::min(),    "2.2250738585072014e-308") );
	REQUIRE(testFromString<double>(std::numeric_limits<double>::lowest(), "-1.7976931348623158e+308") );
	REQUIRE(testFromString<double>(std::numeric_limits<double>::max(),    "1.7976931348623158e+308") );
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

TEST_CASE("fromString int32_t", "[string]")
{
	REQUIRE(testFromString(1389,   "1389") );
	REQUIRE(testFromString(1389,   "  1389") );
	REQUIRE(testFromString(1389,   "+1389") );
	REQUIRE(testFromString(-1389,  "-1389") );
	REQUIRE(testFromString(-1389,  " -1389") );
	REQUIRE(testFromString(555333, "555333") );
	REQUIRE(testFromString(-21,    "-021") );
}

TEST_CASE("StringView", "[string]")
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

TEST_CASE("Trim", "[string]")
{
	REQUIRE(bx::strLTrim("a", "a").isEmpty() );
	REQUIRE(0 == bx::strCmp(bx::strLTrim("aba", "a"), "ba") );

	REQUIRE(bx::strRTrim("a", "a").isEmpty() );
	REQUIRE(0 == bx::strCmp(bx::strRTrim("aba", "a"), "ab") );

	REQUIRE(bx::strTrim("a", "a").isEmpty() );
	REQUIRE(0 == bx::strCmp(bx::strTrim("aba", "a"), "b") );

	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "ab"), "vgd") );

	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "vagbd"), "") );
	REQUIRE(0 == bx::strCmp(bx::strTrimPrefix("abvgd", "vagbd"), "abvgd") );

	REQUIRE(0 == bx::strCmp(bx::strLTrim("abvgd", "vgd"), "abvgd") );
	REQUIRE(0 == bx::strCmp(bx::strLTrim("/555333/podmac/", "/"), "555333/podmac/") );

	REQUIRE(0 == bx::strCmp(bx::strRTrim("abvgd", "vagbd"), "") );
	REQUIRE(0 == bx::strCmp(bx::strTrimSuffix("abvgd", "vagbd"), "abvgd") );

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

TEST_CASE("TrimSpace", "[string]")
{
	REQUIRE(bx::strLTrimSpace("").isEmpty() );
	REQUIRE(bx::strRTrimSpace("").isEmpty() );
	REQUIRE(bx::strTrimSpace( "").isEmpty() );

	REQUIRE(bx::strLTrimSpace("\n").isEmpty() );
	REQUIRE(bx::strRTrimSpace("\n").isEmpty() );
	REQUIRE(bx::strTrimSpace( "\n").isEmpty() );

	const bx::StringView t0("1389");
	const bx::StringView t1("    1389");
	const bx::StringView t2("1389    ");
	const bx::StringView t3("  1389  ");

	REQUIRE(0 == bx::strCmp(bx::strLTrimSpace(t0), t0) );
	REQUIRE(0 == bx::strCmp(bx::strLTrimSpace(t1), t0) );
	REQUIRE(0 == bx::strCmp(bx::strLTrimSpace(t2), t2) );
	REQUIRE(0 == bx::strCmp(bx::strLTrimSpace(t3), "1389  ") );

	REQUIRE(0 == bx::strCmp(bx::strRTrimSpace(t0), t0) );
	REQUIRE(0 == bx::strCmp(bx::strRTrimSpace(t1), t1) );
	REQUIRE(0 == bx::strCmp(bx::strRTrimSpace(t2), t0) );
	REQUIRE(0 == bx::strCmp(bx::strRTrimSpace(t3), "  1389") );

	REQUIRE(0 == bx::strCmp(bx::strTrimSpace(t0), t0) );
	REQUIRE(0 == bx::strCmp(bx::strTrimSpace(t1), t0) );
	REQUIRE(0 == bx::strCmp(bx::strTrimSpace(t2), t0) );
	REQUIRE(0 == bx::strCmp(bx::strTrimSpace(t3), t0) );
}

TEST_CASE("strWord", "[string]")
{
	REQUIRE(bx::strWord(" abvgd-1389.0").isEmpty() );
	REQUIRE(0 == bx::strCmp(bx::strWord("abvgd-1389.0"), "abvgd") );
}

TEST_CASE("strFindBlock", "[string]")
{
	const bx::StringView test0("{ { {} {} abvgd; {} } }");
	const bx::StringView test1(test0, 1, INT32_MAX);

	bx::StringView result = bx::strFindBlock(test1, '{', '}');
	REQUIRE(19 == result.getLength() );
}

TEST_CASE("prefix", "[string]")
{
	REQUIRE( bx::hasPrefix("abvgd-1389.0", "abv") );
	REQUIRE(!bx::hasPrefix("abvgd-1389.0", "bvg") );
	REQUIRE( bx::hasPrefix("abvgd-1389.0", "") );

	REQUIRE(0 == bx::strCmp(bx::strTrimPrefix("abvgd-1389.0", "abv"), "gd-1389.0") );
	REQUIRE(0 == bx::strCmp(bx::strTrimPrefix("abvgd-1389.0", "xyz"), "abvgd-1389.0") );
}

TEST_CASE("suffix", "[string]")
{
	REQUIRE( bx::hasSuffix("abvgd-1389.0", "389.0") );
	REQUIRE(!bx::hasSuffix("abvgd-1389.0", "1389") );
	REQUIRE( bx::hasSuffix("abvgd-1389.0", "") );

	REQUIRE(0 == bx::strCmp(bx::strTrimSuffix("abvgd-1389.0", "389.0"), "abvgd-1") );
	REQUIRE(0 == bx::strCmp(bx::strTrimSuffix("abvgd-1389.0", "xyz"), "abvgd-1389.0") );
}

TEST_CASE("0terminated", "[string]")
{
	const bx::StringView t0("1389");
	REQUIRE(t0.is0Terminated() );
	const bx::StringView t1(strTrimPrefix(t0, "13") );
	REQUIRE(!t1.is0Terminated() );
	const bx::StringView t2(strTrimSuffix(t0, "89") );
	REQUIRE(!t2.is0Terminated() );

	bx::DefaultAllocator crt;
	g_allocator = &crt;

	typedef bx::StringT<&g_allocator> String;

	String st;
	REQUIRE(st.is0Terminated() );

	st = strTrimPrefix(t0, "13");
	REQUIRE(2 == st.getLength() );
	REQUIRE(st.is0Terminated() );

	st = strTrimSuffix(t0, "89");
	REQUIRE(2 == st.getLength() );
	REQUIRE(st.is0Terminated() );
}
