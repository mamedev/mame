#include <bitset>

#include <catch2/catch_test_macros.hpp>
#include <rapidfuzz/string_metric.hpp>

namespace string_metric = rapidfuzz::string_metric;

void validate_bitvector_word(const std::vector<int>& a, std::bitset<64> b)
{
    std::bitset<64> bit_a(0);
    for (int i = 0; i < a.size(); ++i)
    {
        bit_a[i] = a[i];
    }

    REQUIRE(bit_a == b);
}

/**
 * @name JaroWinklerFlagCharsTest
 */
TEST_CASE("JaroWinklerTest")
{
    std::array<std::string, 19> names = {
        "james",
        "robert",
        "john",
        "michael",
        "william",
        "david",
        "joseph",
        "thomas",
        "charles",
        "mary",
        "patricia",
        "jennifer",
        "linda",
        "elizabeth",
        "barbara",
        "susan",
        "jessica",
        "sarah",
        "karen"
    };

    SECTION("testFlagChars")
    {
        for (const auto& name1 : names)
        {
            rapidfuzz::string_view P(name1);
            rapidfuzz::common::PatternMatchVector PM(P);

            for (const auto& name2 : names)
            {
                rapidfuzz::string_view T(name2);

                auto flagged_original = string_metric::detail::flag_similar_characters_original(P, T);
                auto flagged_bitparallel = string_metric::detail::flag_similar_characters_word(PM, P, T);

                INFO("Name1: " << name1 << ", Name2: " << name2);

                validate_bitvector_word(flagged_original.P_flag, flagged_bitparallel.P_flag);
                validate_bitvector_word(flagged_original.T_flag, flagged_bitparallel.T_flag);
                REQUIRE(flagged_original.CommonChars == flagged_bitparallel.CommonChars);
            }    
        }
    }

    SECTION("testFullResult")
    {
        for (const auto& name1 : names)
        {
            rapidfuzz::string_view P(name1);

            for (const auto& name2 : names)
            {
                rapidfuzz::string_view T(name2);

                double Sim_original = string_metric::detail::jaro_similarity_original(P, T, 0);
                double Sim_bitparallel = string_metric::detail::jaro_similarity_word(P, T, 0);

                INFO("Name1: " << name1 << ", Name2: " << name2);
                REQUIRE(Sim_original == Sim_bitparallel);
            }    
        }
    }

    SECTION("testFullResultWithScoreCutoff")
    {
        for (const auto& name1 : names)
        {
            rapidfuzz::string_view P(name1);

            for (const auto& name2 : names)
            {
                rapidfuzz::string_view T(name2);

                double Sim_original = string_metric::detail::jaro_similarity_original(P, T, 90);
                double Sim_bitparallel = string_metric::detail::jaro_similarity_word(P, T, 90);

                INFO("Name1: " << name1 << ", Name2: " << name2);
                REQUIRE(Sim_original == Sim_bitparallel);
            }    
        }
    }

}