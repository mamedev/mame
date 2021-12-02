#include <catch2/catch_test_macros.hpp>

#include <rapidfuzz/utils.hpp>
#include <rapidfuzz/fuzz.hpp>

namespace fuzz = rapidfuzz::fuzz;
namespace utils = rapidfuzz::utils;

/**
 * @name RatioTest
 *
 * @todo Enable 'testPartialTokenSortRatio' once the function is implemented
 */
TEST_CASE("RatioTest")
{
    const std::string s1 = "new york mets";
    const std::string s2 = "new YORK mets";
    const std::string s3 = "the wonderful new york mets";
    const std::string s4 = "new york mets vs atlanta braves";
    const std::string s5 = "atlanta braves vs new york mets";
    const std::string s6 = "new york mets - atlanta braves";
    const std::string s7 = "new york city mets - atlanta braves";
    // test silly corner cases
    const std::string s8 = "{";
    const std::string s8a = "{";
    const std::string s9 = "{a";
    const std::string s9a = "{a";
    const std::string s10 = "a{";
    const std::string s10a = "{b";

    SECTION("testEqual")
    {
        REQUIRE(100 == fuzz::ratio(s1, s1));
		REQUIRE(100 == fuzz::ratio("test", "test"));
        REQUIRE(100 == fuzz::ratio(s8, s8a));
        REQUIRE(100 == fuzz::ratio(s9, s9a));
    }

    SECTION("testCaseInsensitive")
    {
        REQUIRE( 100 != fuzz::ratio(s1, s2) );
        REQUIRE( 100 == fuzz::ratio(utils::default_process(s1), utils::default_process(s2)) );
    }

    SECTION("testPartialRatio")
    {
        REQUIRE( 100 == fuzz::partial_ratio(s1, s1) );
        REQUIRE( 100 != fuzz::ratio(s1, s3) );
        REQUIRE( 100 == fuzz::partial_ratio(s1, s3) );
    }

    SECTION("testTokenSortRatio")
    {
        REQUIRE( 100 == fuzz::token_sort_ratio(s1, s1) );
        const std::string s92 = "metss new york hello";
        const std::string s0 = "metss new york hello";
        REQUIRE(fuzz::token_sort_ratio(s92, s0) >90);
    }

    SECTION("testTokenSetRatio")
    {
        REQUIRE( 100 == fuzz::token_set_ratio(s4, s5) );
        REQUIRE( 100 == fuzz::token_set_ratio(s8, s8a, false) );
        REQUIRE( 100 == fuzz::token_set_ratio(s9, s9a, true) );
        REQUIRE( 100 == fuzz::token_set_ratio(s9, s9a, false) );
        REQUIRE( 50  == fuzz::token_set_ratio(s10, s10a, false) );
    }

    SECTION("testPartialTokenSetRatio")
    {
        REQUIRE( 100 == fuzz::partial_token_set_ratio(s4, s7) );
    }

    SECTION("testWRatioEqual")
    {
        REQUIRE( 100 == fuzz::WRatio(s1, s1) );
    }

    SECTION("testWRatioPartialMatch")
    {
        // a partial match is scaled by .9
        REQUIRE( 90 == fuzz::WRatio(s1, s3) );
    }

    SECTION("testWRatioMisorderedMatch")
    {
        // misordered full matches are scaled by .95
        REQUIRE( 95 == fuzz::WRatio(s4, s5) );
    }

    SECTION("testEmptyStringsScore100")
    {
        REQUIRE( 100 == fuzz::ratio("", "") );
        REQUIRE( 100 == fuzz::partial_ratio("", "") );
    }
}