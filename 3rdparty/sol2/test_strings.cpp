#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>

#include <iostream>

struct test {};
template <typename T>
struct test_t {};

namespace muh_namespace {
	struct ns_test {};

	namespace {
		struct ns_anon_test {};
	}
}

// There isn't a single library roundtripping which codecvt works on. We'll do the nitty-gritty of it later...
TEST_CASE("stack/strings", "test that strings can be roundtripped") {
	sol::state lua;

	static const char utf8str[] = "\xF0\x9F\x8D\x8C\x20\xE6\x99\xA5\x20\x46\x6F\x6F\x20\xC2\xA9\x20\x62\x61\x72\x20\xF0\x9D\x8C\x86\x20\x62\x61\x7A\x20\xE2\x98\x83\x20\x71\x75\x78";
	static const char16_t utf16str[] = { 0xD83C, 0xDF4C, 0x20, 0x6665, 0x20, 0x46, 0x6F, 0x6F, 0x20, 0xA9, 0x20, 0x62, 0x61, 0x72, 0x20, 0xD834, 0xDF06, 0x20, 0x62, 0x61, 0x7A, 0x20, 0x2603, 0x20, 0x71, 0x75, 0x78, 0x00 };
	static const char32_t utf32str[] = { 0x1F34C, 0x0020, 0x6665, 0x0020, 0x0046, 0x006F, 0x006F, 0x0020, 0x00A9, 0x0020, 0x0062, 0x0061, 0x0072, 0x0020, 0x1D306, 0x0020, 0x0062, 0x0061, 0x007A, 0x0020, 0x2603, 0x0020, 0x0071, 0x0075, 0x0078, 0x00 };
#ifdef _WIN32
	INFO("win32 widestr");
	static const wchar_t widestr[] = { 0xD83C, 0xDF4C, 0x20, 0x6665, 0x20, 0x46, 0x6F, 0x6F, 0x20, 0xA9, 0x20, 0x62, 0x61, 0x72, 0x20, 0xD834, 0xDF06, 0x20, 0x62, 0x61, 0x7A, 0x20, 0x2603, 0x20, 0x71, 0x75, 0x78, 0x00 };
#else
	INFO("non-windows widestr");
	static const wchar_t widestr[] = { 0x1F34C, 0x0020, 0x6665, 0x0020, 0x0046, 0x006F, 0x006F, 0x0020, 0x00A9, 0x0020, 0x0062, 0x0061, 0x0072, 0x0020, 0x1D306, 0x0020, 0x0062, 0x0061, 0x007A, 0x0020, 0x2603, 0x0020, 0x0071, 0x0075, 0x0078, 0x00 };
#endif
	static const std::string utf8str_s = utf8str;
	static const std::u16string utf16str_s = utf16str;
	static const std::u32string utf32str_s = utf32str;
	static const std::wstring widestr_s = widestr;

#ifdef SOL_CODECVT_SUPPORT
	INFO("sizeof(wchar_t): " << sizeof(wchar_t));
	INFO("sizeof(char16_t): " << sizeof(char16_t));
	INFO("sizeof(char32_t): " << sizeof(char32_t));
	INFO("utf8str: " << utf8str);
	INFO("utf8str_s: " << utf8str_s);

	lua["utf8"] = utf8str;
	lua["utf16"] = utf16str;
	lua["utf32"] = utf32str;
	lua["wide"] = widestr;

	std::string utf8_to_utf8 = lua["utf8"];
	std::string utf16_to_utf8 = lua["utf16"];
	std::string utf32_to_utf8 = lua["utf32"];
	std::string wide_to_utf8 = lua["wide"];

	REQUIRE(utf8_to_utf8 == utf8str_s);
	REQUIRE(utf16_to_utf8 == utf8str_s);
	REQUIRE(utf32_to_utf8 == utf8str_s);
	REQUIRE(wide_to_utf8 == utf8str_s);
	
	std::wstring utf8_to_wide = lua["utf8"];
	std::wstring utf16_to_wide = lua["utf16"];
	std::wstring utf32_to_wide = lua["utf32"];
	std::wstring wide_to_wide = lua["wide"];

	REQUIRE(utf8_to_wide == widestr_s);
	REQUIRE(utf16_to_wide == widestr_s);
	REQUIRE(utf32_to_wide == widestr_s);
	REQUIRE(wide_to_wide == widestr_s);

	std::u16string utf8_to_utf16 = lua["utf8"];
	std::u16string utf16_to_utf16 = lua["utf16"];
	std::u16string utf32_to_utf16 = lua["utf32"];
	std::u16string wide_to_utf16 = lua["wide"];

	REQUIRE(utf8_to_utf16 == utf16str_s);
	REQUIRE(utf16_to_utf16 == utf16str_s);
	REQUIRE(utf32_to_utf16 == utf16str_s);
	REQUIRE(wide_to_utf16 == utf16str_s);

	std::u32string utf8_to_utf32 = lua["utf8"];
	std::u32string utf16_to_utf32 = lua["utf16"];
	std::u32string utf32_to_utf32 = lua["utf32"];
	std::u32string wide_to_utf32 = lua["wide"];

	REQUIRE(utf8_to_utf32 == utf32str_s);
	REQUIRE(utf16_to_utf32 == utf32str_s);
	REQUIRE(utf32_to_utf32 == utf32str_s);
	REQUIRE(wide_to_utf32 == utf32str_s);

	char32_t utf8_to_char32 = lua["utf8"];
	char32_t utf16_to_char32 = lua["utf16"];
	char32_t utf32_to_char32 = lua["utf32"];
	char32_t wide_to_char32 = lua["wide"];

	REQUIRE(utf8_to_char32 == utf32str[0]);
	REQUIRE(utf16_to_char32 == utf32str[0]);
	REQUIRE(utf32_to_char32 == utf32str[0]);
	REQUIRE(wide_to_char32 == utf32str[0]);
#endif // codecvt support
}

TEST_CASE("detail/demangling", "test some basic demangling cases") {
	std::string teststr = sol::detail::short_demangle<test>();
	std::string nsteststr = sol::detail::short_demangle<muh_namespace::ns_test>();
	std::string nsateststr = sol::detail::short_demangle<muh_namespace::ns_anon_test>();

	REQUIRE(teststr == "test");
	REQUIRE(nsteststr == "ns_test");
	REQUIRE(nsateststr == "ns_anon_test");
}

TEST_CASE("object/string-pushers", "test some basic string pushers with in_place constructor") {
	sol::state lua;

	sol::object ocs(lua, sol::in_place, "bark\0bark", 9);
	sol::object os(lua, sol::in_place<std::string>, std::string("bark\0bark", 9), 8);
	bool test1 = os.as<std::string>() == std::string("bark\0bar", 8);
	bool test2 = ocs.as<std::string>() == std::string("bark\0bark", 9);
	REQUIRE(test1);
	REQUIRE(test2);
}
