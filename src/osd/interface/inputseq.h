// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputseq.h

    A combination of hosts inputs that can be assigned to a control.

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_INPUTSEQ_H
#define MAME_OSD_INTERFACE_INPUTSEQ_H

#pragma once

#include "inputcode.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <utility>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a combination of input_codes, supporting AND/OR and inversion

class input_seq
{
public:
	// construction/destruction
	input_seq() noexcept : input_seq(std::make_index_sequence<std::tuple_size<decltype(m_code)>::value>())
	{
	}
	template <typename... T>
	input_seq(input_code code_0, T... code_n) noexcept :
		input_seq(std::make_index_sequence<std::tuple_size<decltype(m_code)>::value - sizeof...(T) - 1>(), code_0, code_n...)
	{
	}
	constexpr input_seq(const input_seq &rhs) noexcept = default;

	// operators
	bool operator==(const input_seq &rhs) const noexcept { return m_code == rhs.m_code; }
	bool operator!=(const input_seq &rhs) const noexcept { return m_code != rhs.m_code; }
	constexpr input_code operator[](int index) const noexcept { return (index >= 0 && index < m_code.size()) ? m_code[index] : end_code; }
	input_seq &operator+=(input_code code) noexcept;
	input_seq &operator|=(input_code code) noexcept;

	// getters
	constexpr bool empty() const noexcept { return m_code[0] == end_code; }
	constexpr int max_size() const noexcept { return std::tuple_size<decltype(m_code)>::value; }
	int length() const noexcept;
	bool is_valid() const noexcept;
	constexpr bool is_default() const noexcept { return m_code[0] == default_code; }

	// setters
	template <typename... T> void set(input_code code_0, T... code_n) noexcept
	{
		static_assert(sizeof...(T) < std::tuple_size<decltype(m_code)>::value, "too many codes for input_seq");
		set<0>(code_0, code_n...);
	}
	void reset() noexcept { set(end_code); }
	void set_default() noexcept { set(default_code); }
	void backspace() noexcept;
	void replace(input_code oldcode, input_code newcode) noexcept;

	// constant codes used in sequences
	static constexpr input_code end_code { DEVICE_CLASS_INTERNAL, 0, ITEM_CLASS_INVALID, ITEM_MODIFIER_NONE, ITEM_ID_SEQ_END };
	static constexpr input_code default_code { DEVICE_CLASS_INTERNAL, 0, ITEM_CLASS_INVALID, ITEM_MODIFIER_NONE, ITEM_ID_SEQ_DEFAULT };
	static constexpr input_code not_code { DEVICE_CLASS_INTERNAL, 0, ITEM_CLASS_INVALID, ITEM_MODIFIER_NONE, ITEM_ID_SEQ_NOT };
	static constexpr input_code or_code { DEVICE_CLASS_INTERNAL, 0, ITEM_CLASS_INVALID, ITEM_MODIFIER_NONE, ITEM_ID_SEQ_OR };

	// constant sequences
	static const input_seq empty_seq;

private:
	static constexpr input_code get_end_code(size_t) noexcept { return end_code; }

	template <size_t... N, typename... T>
	input_seq(std::integer_sequence<size_t, N...>, T... code) noexcept : m_code({ code..., get_end_code(N)... })
	{
	}
	template <size_t... N>
	input_seq(std::integer_sequence<size_t, N...>) noexcept : m_code({ get_end_code(N)... })
	{
	}

	template <unsigned N> void set() noexcept
	{
		std::fill(std::next(m_code.begin(), N), m_code.end(), end_code);
	}
	template <unsigned N, typename... T> void set(input_code code_0, T... code_n) noexcept
	{
		m_code[N] = code_0;
		set<N + 1>(code_n...);
	}

	// internal state
	std::array<input_code, 16> m_code;
};

}  // namespace osd

#endif // MAME_OSD_INTERFACE_INPUTSEQ_H
