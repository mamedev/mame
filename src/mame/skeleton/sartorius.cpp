// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Sartorius 3733 digital scale

 Mainboard:
 * 4194.304 kHz crystal
 * Intel D4040 4-bit CPU
 * Intel D4289 8-bit memory controller
 * 2 * Intel P4002-1 (64+16)*4 RAM (sockets for 2 * Intel P4002-2 unpopulated)
 * 2 * 2708 1k*8 ROM labelled "129/0" and "129/1"
 * National Semiconductor MM74C907N hex open-drain P-channel buffer
 * Intersil IM6561AIJN 256*4 CMOS RAM
 * RCA CD4007AE dual complementary pair plus inverter
 * National Semicondictor CD4049CN hex inverting buffer
 * Intel P4201A clock generator
 * 4 * Motorola MC14099B 8-bit addressable latch
 * 2 * RCA CD4052BE 2-channel 4:1 analog switch

 Display board (connects to mainboard via 28-pin edge connector):
 * MM74C42 1-of-10 decoder
 * RCA CD4071BE Quad 2-input OR gate
 * RCA CD4011BE Quad 2-input NAND gate
 * RCA CD4055BE BCD to 7-segment decoder with high-voltage outputs
 * 12 TO-92 transistors near the MM74C42 and CD4071BE
 * 7 TO-92 transistors near the CD4011BE
 * 7 TO-92 transistors near the CD4055BE

 Front panel has a digital display, an "R" button for switching between
 300g and 3kg ranges, and a T button for zeroing.  Rear panel has a
 three-position rotary switch to select between standalone operation and
 data acquisition modes, and a 50-pin micro ribbon connector for data
 communication.

 The vacuum fluorescent display has seven 7-segment digits, each of
 which can be followed by a . (stop) or , (comma) for digit grouping or
 decimal point.  The last digit can also show the vulgar fraction ½
 using the G segment as the fraction bar and additional 1 and 2 segments
 (these could potentially be used independently as well).  To the right
 of the least significant digit, the display can show % (percent) or g
 (lowecase G).  To the legt of the leftmost digit, the display has,
 from top to bottom, + (plus), - (minus), and a small circle.

 Strangely, the P4201A clock generator IC is a long way from the
 crystal.  The mode pin is tied to Vss, putting it in divide-by-seven
 mode.

 No dates or other ASCII strings in program ROM.  IC date codes up to
 1978.
 */

#include "emu.h"
#include "cpu/mcs40/mcs40.h"

namespace {

class sart3733_state : public driver_device
{
public:
	sart3733_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{
	}

	void sart3733(machine_config &config);

private:
	void sart3733_memory(address_map &map) ATTR_COLD;
	void sart3733_rom(address_map &map) ATTR_COLD;
	void sart3733_status(address_map &map) ATTR_COLD;
};


	void sart3733_state::sart3733_rom(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("maincpu", 0x0000);
}

	void sart3733_state::sart3733_memory(address_map &map)
{
	map(0x0000, 0x007f).ram(); // 2 * 4002
}

	void sart3733_state::sart3733_status(address_map &map)
{
	map(0x0000, 0x001f).ram(); // 2 * 4002
}


void sart3733_state::sart3733(machine_config &config)
{
	i4040_cpu_device &cpu(I4040(config, "maincpu", 750'000)); // clock speed is a complete guess - can't see crystal or multi-phase clock generator
	cpu.set_rom_map(&sart3733_state::sart3733_rom);
	cpu.set_ram_memory_map(&sart3733_state::sart3733_memory);
	cpu.set_ram_status_map(&sart3733_state::sart3733_status);
}


INPUT_PORTS_START(sart3733)
INPUT_PORTS_END


ROM_START (sart3733)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("129-0", 0x0000, 0x0400, CRC(7ba30b9a) SHA1(40f828145542c2032d30b9921c829406ba9a3679))
	ROM_LOAD("129-1", 0x0400, 0x0400, CRC(a1e2ac6e) SHA1(aaa504274bc077a0de2ba78675d1eb3ad3157b6a))
ROM_END

} // anonymous namespace

SYST( 1978?, sart3733, 0, 0, sart3733, sart3733, sart3733_state, empty_init, "Sartorius-Werke GmbH", "3733 (digital scale)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
