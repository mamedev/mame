// license:BSD-3-Clause
// copyright-holders:Jon Guidry
#ifndef MAME_BUS_TI99_GROMPORT_GIGACART_H
#define MAME_BUS_TI99_GROMPORT_GIGACART_H

#pragma once

#include <cstdint>
#include <string_view>

namespace bus::ti99::gromport::gigacart {

// Geometry is independent of boot hardware and the installed ROM capacity.
constexpr uint32_t bank_count(unsigned data_bits) { return 1U << (12 + data_bits); }
constexpr uint32_t select(uint16_t address, uint8_t data, unsigned data_bits)
{
	return ((uint32_t(data) & ((1U << data_bits) - 1)) << 12) | ((address >> 1) & 0x0fff);
}
constexpr uint64_t image_offset(uint32_t bank, uint16_t address, uint64_t size)
{
	return (uint64_t(bank) * 8192 + (address & 0x1fff)) & (size - 1);
}
constexpr bool valid_size(uint64_t size, unsigned data_bits)
{
	return size >= 8192 && !(size & (size - 1)) && size <= uint64_t(bank_count(data_bits)) * 8192;
}
inline bool parse_width(std::string_view value, unsigned &bits)
{
	if (value.size() != 1 || value[0] < '1' || value[0] > '8') return false;
	bits = value[0] - '0';
	return true;
}
inline bool parse_initial(std::string_view value, unsigned bits, uint32_t &bank)
{
	bank = 0;
	if (value == "first") return true;
	if (value == "last") { bank = bank_count(bits) - 1; return true; }
	if (value.empty()) return false;
	for (char ch : value)
	{
		if (ch < '0' || ch > '9') return false;
		bank = bank * 10 + ch - '0';
		if (bank >= bank_count(bits)) return false;
	}
	return true;
}

} // namespace bus::ti99::gromport::gigacart
#endif
