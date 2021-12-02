/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#include <cassert>
#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <rapidfuzz/details/common.hpp>
#include <stdexcept>

namespace rapidfuzz {
namespace string_metric {
namespace detail {

template <typename CharT1, typename CharT2>
std::vector<std::size_t> levenshtein_matrix(basic_string_view<CharT1> s1,
                                            basic_string_view<CharT2> s2)
{
    std::size_t rows = s1.size() + 1;
    std::size_t cols = s2.size() + 1;
    std::size_t matrix_size = rows * cols;
    if (matrix_size / rows != cols) {
        throw std::length_error("cannot create matrix larger than SIZE_MAX");
    }
    std::vector<std::size_t> matrix(rows * cols);

    for (std::size_t col = 0; col < cols; col++) {
        matrix[col] = col;
    }

    for (std::size_t row = 1; row < rows; row++) {
        matrix[row * cols] = row;
    }

    if (s2.empty()) {
        return matrix;
    }

    for (std::size_t i = 0; i < s1.size(); i++) {
        size_t* prev = &matrix[i * cols];
        size_t* cur = &matrix[(i + 1) * cols + 1];
        auto char1 = s1[i];
        size_t temp = i;

        for (const auto& char2 : s2) {
            temp = std::min({temp + 1, *prev + common::mixed_sign_unequal(char1, char2), *(prev + 1) + 1});
            *cur = temp;
            cur++;
            prev++;
        }
    }

    return matrix;
}

template <typename CharT1, typename CharT2>
std::vector<LevenshteinEditOp> levenshtein_editops(basic_string_view<CharT1> s1,
                                                   basic_string_view<CharT2> s2)
{
    /* prefix and suffix are no-ops, which do not need to be added to the editops */
    auto affix = common::remove_common_affix(s1, s2);
    std::vector<std::size_t> matrix = levenshtein_matrix(s1, s2);
    std::size_t dist = matrix.back();

    std::vector<LevenshteinEditOp> editops(dist);

    if (dist == 0) {
        return editops;
    }

    std::size_t row = s1.size();
    std::size_t col = s2.size();
    std::size_t cols = s2.size() + 1;
    const std::size_t* cur = &matrix.back();

    while (row || col) {
        /* horizontal == current and character similar -> no-operation */
        if (row && col && (*cur == *(cur - cols - 1)) && common::mixed_sign_equal(s1[row - 1], s2[col - 1])) {
            row--;
            col--;
            cur -= cols + 1;
            continue;
        }
        /* horizontal + 1 == current -> replacement */
        else if (row && col && (*cur == *(cur - cols - 1) + 1)) {
            dist--;
            row--;
            col--;
            editops[dist].type = LevenshteinEditType::Replace;
            editops[dist].src_pos = row + affix.prefix_len;
            editops[dist].dest_pos = col + affix.prefix_len;
            cur -= cols + 1;
        }
        /* left + 1 == current -> insertion */
        else if (col && (*cur == *(cur - 1) + 1)) {
            dist--;
            col--;
            editops[dist].type = LevenshteinEditType::Insert;
            editops[dist].src_pos = row + affix.prefix_len;
            editops[dist].dest_pos = col + affix.prefix_len;
            cur--;
        }
        /* above + 1 == current -> deletion */
        else {
            /* this should be the case as long as there is no error in the implementation */
            assert((row && (*cur == *(cur - cols) + 1)));

            dist--;
            row--;
            editops[dist].type = LevenshteinEditType::Delete;
            editops[dist].src_pos = row + affix.prefix_len;
            editops[dist].dest_pos = col + affix.prefix_len;
            cur -= cols;
        }
    }

    return editops;
}

} // namespace detail
} // namespace string_metric
} // namespace rapidfuzz
