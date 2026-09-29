// license:BSD-3-Clause
// copyright-holders:Jon Guidry
#ifdef GIGACART_STANDALONE
#define CATCH_CONFIG_MAIN
#endif
#include "catch.hpp"
#include "../../src/devices/bus/ti99/gromport/gigacart.h"

using namespace bus::ti99::gromport;

TEST_CASE("Gigacart bank bus and capacity", "[gigacart]")
{
	for (unsigned bits = 1; bits <= 8; ++bits)
	{
		uint32_t const count = gigacart::bank_count(bits);
		uint64_t const size = uint64_t(count) * 8192;
		REQUIRE(gigacart::valid_size(size, bits));
		REQUIRE_FALSE(gigacart::valid_size(size * 2, bits));
		for (uint32_t bank = 0; bank < count; ++bank)
		{
			uint16_t const address = 0x6000 | ((bank & 4095) * 2);
			uint8_t const data = bank >> 12;
			if (gigacart::select(address, data, bits) != bank || gigacart::select(address | 1, data, bits) != bank)
				FAIL("Bank selection differs from bus vector");
		}
	}
	REQUIRE(gigacart::select(0x6000, 0xfd, 2) == 0x1000);
	REQUIRE(gigacart::select(0x6000, 0xfe, 2) == 0x2000);
	REQUIRE(gigacart::select(0x7ffe, 0xff, 2) == 0x3fff);
	// TI MOV >0102,@>6000: odd byte 02 first, then even byte 01.
	REQUIRE(gigacart::select(0x6001, 2, 2) == 0x2000);
	REQUIRE(gigacart::select(0x6000, 1, 2) == 0x1000);
	REQUIRE(gigacart::image_offset(0xfffff, 0x7fff, UINT64_C(0x200000000)) == UINT64_C(0x1ffffffff));
	REQUIRE(gigacart::image_offset(0x80000, 0x6000, UINT64_C(0x200000000)) == UINT64_C(0x100000000));
	REQUIRE(gigacart::image_offset(0xfffff, 0x7fff, 0x4000) == 0x3fff);
	REQUIRE_FALSE(gigacart::valid_size(0, 2));
	REQUIRE_FALSE(gigacart::valid_size(8191, 2));
	REQUIRE_FALSE(gigacart::valid_size(24576, 2));
}

TEST_CASE("Gigacart configuration is explicit", "[gigacart]")
{
	unsigned bits = 0;
	for (auto value : { "", "0", "9", "02", "-1", "2x", " 2", "2 " })
		REQUIRE_FALSE(gigacart::parse_width(value, bits));
	REQUIRE(gigacart::parse_width("2", bits));
	REQUIRE(bits == 2);
	uint32_t bank;
	REQUIRE(gigacart::parse_initial("first", 2, bank));
	REQUIRE(bank == 0);
	REQUIRE(gigacart::parse_initial("last", 2, bank));
	REQUIRE(bank == 16383);
	REQUIRE(gigacart::parse_initial("4096", 2, bank));
	REQUIRE(bank == 4096);
	for (auto value : { "", "-1", "+1", "0x10", "16384", "999999999999999999999999999" })
		REQUIRE_FALSE(gigacart::parse_initial(value, 2, bank));
}
