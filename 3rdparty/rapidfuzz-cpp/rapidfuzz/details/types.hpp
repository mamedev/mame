/* SPDX-License-Identifier: MIT */
/* Copyright © 2020 Max Bachmann */

#pragma once

#include <type_traits>
#include <vector>

#include <rapidfuzz/details/string_view.hpp>

namespace rapidfuzz {

/* 0.0% - 100.0% */
using percent = double;

template <typename CharT>
using string_view_vec = std::vector<basic_string_view<CharT>>;

struct StringAffix {
    std::size_t prefix_len;
    std::size_t suffix_len;
};

struct LevenshteinWeightTable {
    std::size_t insert_cost;
    std::size_t delete_cost;
    std::size_t replace_cost;
};

/**
 * @brief Edit operation types used by the Levenshtein distance
 */
enum class LevenshteinEditType {
    None = 0,    /**< No Operation required */
    Replace = 1, /**< Replace a character if a string by another character */
    Insert = 2,  /**< Insert a character into a string */
    Delete = 3   /**< Delete a character from a string */
};

/**
 * @brief Edit operations used by the Levenshtein distance
 *
 * This represents an edit operation of type type which is applied to
 * the source string
 *
 * Replace: replace character at src_pos with character at dest_pos
 * Insert:  insert character from dest_pos at src_pos
 * Delete:  delete character at src_pos
 */
struct LevenshteinEditOp {
    LevenshteinEditType type; /**< type of the edit operation */
    std::size_t src_pos;      /**< index into the source string */
    std::size_t dest_pos;     /**< index into the destination string */
};

static inline bool operator ==(LevenshteinEditOp a, LevenshteinEditOp b) {
	return (a.type == b.type) && (a.src_pos == b.src_pos) && (a.dest_pos == b.dest_pos);
}

} // namespace rapidfuzz
