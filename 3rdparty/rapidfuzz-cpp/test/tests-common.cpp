#include <catch2/catch_test_macros.hpp>

#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/fuzz.hpp>
#include <rapidfuzz/details/string_view.hpp>

#include <string>
#include <vector>

namespace common = rapidfuzz::common;

TEST_CASE("to_string_view")
{
	auto R0 =   R"("Hello \ world")"; // const char*
    auto R1 = u8R"("Hello \ world")"; // const char*, encoded as UTF-8
    auto R2 =  LR"("Hello \ world")"; // const wchar_t*
    auto R3 =  uR"("Hello \ world")"; // const char16_t*, encoded as UTF-16
    auto R4 =  UR"("Hello \ world")"; // const char32_t*, encoded as UTF-32

    const std::string str = "new york mets";
	const std::wstring wstr = L"new york mets";

    SECTION("test string literals as input")
    {
        auto str_view = common::to_string_view(R0);
        REQUIRE(R0 == str_view.data());
		REQUIRE(std::char_traits<char>::length(R0) == str_view.size());

        auto str_view1 = common::to_string_view(R1);
        REQUIRE(R1 == str_view1.data());
		REQUIRE(std::char_traits<char>::length(R1) == str_view1.size());

        auto str_view2 = common::to_string_view(R2);
        REQUIRE(R2 == str_view2.data());
		REQUIRE(std::char_traits<wchar_t>::length(R2) == str_view2.size());

        auto str_view3 = common::to_string_view(R3);
        REQUIRE(R3 == str_view3.data());
		REQUIRE(std::char_traits<char16_t>::length(R3) == str_view3.size());

        auto str_view4 = common::to_string_view(R4);
        REQUIRE(R4 == str_view4.data());
		REQUIRE(std::char_traits<char32_t>::length(R4) == str_view4.size());
    }

    SECTION("test strings as input")
    {
		auto str_view = common::to_string_view(str);
        REQUIRE(str.data() == str_view.data());
		REQUIRE(str.size() == str_view.size());

		auto wstr_view = common::to_string_view(wstr);
        REQUIRE(wstr.data() == wstr_view.data());
		REQUIRE(wstr.size() == wstr_view.size());
    }

    SECTION("test vectors as input")
    {
        const std::vector<char> vec_str(str.begin(), str.end());
	    const std::vector<wchar_t> vec_wstr(wstr.begin(), wstr.end());
		const std::vector<int> vec_int = { 1, 10, 99};

		auto str_view = common::to_string_view(vec_str);
        REQUIRE(vec_str.data() == str_view.data());
		REQUIRE(vec_str.size() == str_view.size());

		auto wstr_view = common::to_string_view(vec_wstr);
        REQUIRE(vec_wstr.data() == wstr_view.data());
		REQUIRE(vec_wstr.size() == wstr_view.size());

		auto int_view = common::to_string_view(vec_int);
        REQUIRE(vec_int.data() == int_view.data());
		REQUIRE(vec_int.size() == int_view.size());
    }

    SECTION("test rapidfuzz::string_view as input")
    {
        const rapidfuzz::string_view view_str = str;
		const rapidfuzz::wstring_view view_wstr = wstr;

		auto str_view = common::to_string_view(view_str);
        REQUIRE(view_str.data() == str_view.data());
		REQUIRE(view_str.size() == str_view.size());

		auto wstr_view = common::to_string_view(view_wstr);
        REQUIRE(view_wstr.data() == wstr_view.data());
		REQUIRE(view_wstr.size() == wstr_view.size());
    }
}