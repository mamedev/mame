// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    LEAPPAD:
    Example-Video: https://www.youtube.com/watch?v=LtUhENu5TKc
    The LEAPPAD is basically compareable to the SEGA PICO, but without
    Screen-Output! Each "Game" consists of two parts (Book + Cartridge).
    Insert the cartridge into the system and add the Book on the Top of the
    "console" and you can click on each pages and hear sounds or
    learning-stuff on each page...

    MY FIRST LEAPPAD:
    Basically the same as the LEAPPAD, but for even younger kids! (Cartridge
    internal PCB's are identical to LEAPPAD).
    Example Video: https://www.youtube.com/watch?v=gsf8XYV1Tpg

    LITTLE TOUCH LEAPPAD:
    Same as the other LEAPPAD models, but aimed at babies.

    Don't get confused by the name "LEAPPAD", as it looks like Leapfrog
    also released some kind of Tablet with this name, and they even released
    a new "LEAPPAD" in around 2016:
    https://www.youtube.com/watch?v=MXFSgj6xLTU , which nearly looks like the
    same, but is most likely techically completely different...

    The cartridges pinout is the same on the three systems:
       A1  N/C (A21?)
       A2  A20
       A3  A19
       A4  A8
       A5  A9
       A6  A6
       A7  A5
       A8  A4
       A9  A3
      A10  A2
      A11  A1
      A12  A0
      A13  N/C (R/W? /CE2?)
      A14  /CE
      A15  /OE
      A16  D0
      A17  D1
      A18  D2
      A19  D3
      A20  VCC
      B1   N/C (A22?)
      B2   N/C (A23?)
      B3   A18
      B4   A17
      B5   A7
      B6   GND
      B7   A10
      B8   A11
      B9   A12
      B10  A13
      B11  A14
      B12  A15
      B13  A16
      B14  GND
      B15  A-1
      B16  D7
      B17  D6
      B18  D5
      B19  D4
      B20  GND

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class leapfrog_leappad_state : public driver_device
{
public:
	leapfrog_leappad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void leapfrog_leappad(machine_config &config);
	void leapfrog_mfleappad(machine_config &config);
	void leapfrog_ltleappad(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};



void leapfrog_leappad_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void leapfrog_leappad_state::machine_reset()
{
}

void leapfrog_leappad_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0x10000); // TODO: banking
}

void leapfrog_leappad_state::ext_map(address_map &map)
{
}

DEVICE_IMAGE_LOAD_MEMBER(leapfrog_leappad_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( leapfrog_leappad )
INPUT_PORTS_END


void leapfrog_leappad_state::leapfrog_leappad(machine_config &config)
{
	I8032(config, m_maincpu, 96000000/10); //  LeapPad Leapfrog 05-9-01 FS80A363  (which exact type is it?)
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_leappad_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_leappad_state::ext_map);

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_leappad_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_leappad_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_leappad_cart");
}

void leapfrog_leappad_state::leapfrog_mfleappad(machine_config &config)
{
	I8032(config, m_maincpu, 96000000/10); //  LeapPad Leapfrog 05-9-01 FS80A363  (which exact type is it?)
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_leappad_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_leappad_state::ext_map);

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_mfleappad_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_leappad_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_mfleappad_cart");
}

void leapfrog_leappad_state::leapfrog_ltleappad(machine_config &config)
{
	I8032(config, m_maincpu, 96000000/10); // (which exact type is it?)
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_leappad_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_leappad_state::ext_map);

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_ltleappad_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_leappad_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_ltleappad_cart");
}


// All of these contain the string "Have you copied our ROM?" near the date codes

ROM_START( leappad )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("ila2_universal")
	ROM_SYSTEM_BIOS( 0, "ila2_universal",  "Universal" )
	ROMX_LOAD( "leappadbios.bin",       0x000000, 0x100000, CRC(c886cddc) SHA1(f8a83b156feb28315d2321758678e141600a0d4e), ROM_BIOS(0) ) // contains "Aug 06 2001.16:33:16.155-00450.LeapPad ILA2 Universal Base ROM" and "Copyright (c) 1998-2001 Knowledge Kids Enterprises, Inc."
	ROM_SYSTEM_BIOS( 1, "2mb_canada_full", "Canada" )
	ROMX_LOAD( "leappadbioscanada.bin", 0x000000, 0x200000, CRC(cc12e3db) SHA1(adf52232adcfd4de5d8e31c0e0c09be61718a9d4), ROM_BIOS(1) ) // contains "Jan 23 2004 11:28:40 152-10620 2MB Canada Full Base ROM" and "Copyright (c) 2000-2004 LeapFrog Enterprises, Inc."

	ROM_REGION( 0x8000, "bootloader", 0) // Main MCU (LeapFrog FS80A363) internal ROM (exact size unknown)
	ROM_LOAD( "fs80a363.u4", 0x0000, 0x8000, NO_DUMP )
ROM_END

ROM_START( mfleappad )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("internat_v1.3")
	ROM_SYSTEM_BIOS( 0, "internat_v1.3",  "International" )
	ROMX_LOAD( "myfirstleappadinternational.bin", 0x000000, 0x100000, CRC(4dc0c4d5) SHA1(573ecf2efaccf70e619cf54d63be9169e469ee6f), ROM_BIOS(0) ) // contains "May 07 2002 10:53:14 152-00932 MFLP International base ROM V1.3" and "Copyright (c) 2002 LeapFrog Enterprises, Inc."
	ROM_SYSTEM_BIOS( 1, "us_2004",        "US" )
	ROMX_LOAD( "myfirstleappadbios.bin",          0x000000, 0x400000, CRC(19174c16) SHA1(e0ba644fdf38fd5f91ab8c4b673c4a658cc3e612), ROM_BIOS(1) ) // contains "Feb 13 2004.10:58:53.152-10573.MFLP US Base ROM - 2004" and "Copyright (c) 2004 LeapFrog Enterprises, Inc."

	ROM_REGION( 0x8000, "bootloader", 0) // Main MCU (LeapFrog FS80A363) internal ROM (exact size unknown)
	ROM_LOAD( "fs80a363.u4", 0x0000, 0x8000, NO_DUMP )
ROM_END

ROM_START( leappadmic )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("us_2004")
	ROM_SYSTEM_BIOS( 0, "us_2004", "US" )
	ROMX_LOAD( "leapfrogvpadwithmic.bin", 0x000000, 0x800000, CRC(c289b660) SHA1(fa163661260942eab92e34ae802b3da0a6130a39), ROM_BIOS(0) ) // contains "Apr 29 2004 12:09:09 152-10793 MIB - LeapPad Plus Microphone baseROM - US 2004" and "Copyright (c) 2000-2004 LeapFrog Enterprises, Inc."

	ROM_REGION( 0x8000, "bootloader", 0) // Main MCU (LeapFrog FS80A363) internal ROM (exact size unknown)
	ROM_LOAD( "fs80a363.u4", 0x0000, 0x8000, NO_DUMP )

	// Sunplus PA7790 sound MCU internal ROM (exact size unknown)
	// It is unknown if other LeapPad models use the same MCU
	ROM_REGION( 0x8000, "soundmcu", 0)
	ROM_LOAD( "pa7790.u15",  0x0000, 0x8000, NO_DUMP )
ROM_END

ROM_START( ltleappad )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mar_10_2005")
	ROM_SYSTEM_BIOS( 0, "mar_10_2005", "Universal" ) // Includes the game 'One Bear in the Bedroom'
	ROMX_LOAD( "littletouchleappadbios.bin",      0x000000, 0x400000, CRC(13687b26) SHA1(6ec1a47aaef9c9ed134bb143c2631f4d89d7c236), ROM_BIOS(0) ) // contains "Mar 10 2005 07:01:53 152-11244" and "Copyright (c) 2002-2005 LeapFrog Enterprises, Inc."
	ROM_SYSTEM_BIOS( 1, "germany",     "Germany" )
	ROMX_LOAD( "leappad_little_touch_german.bin", 0x000000, 0x400000, CRC(39ee76a2) SHA1(34f1b6e075e10e14380d925944f4c84d068ec58e), ROM_BIOS(1) ) // contains "Jan 11 2005 10:45:42 152-11010 Full Base ROM: V1.0 - Germany"

	ROM_REGION( 0x8000, "bootloader", 0) // Main MCU (LeapFrog FS80A363) internal ROM (exact size unknown)
	ROM_LOAD( "fs80a363.u4", 0x0000, 0x8000, NO_DUMP )
ROM_END

} // anonymous namespace


//    year, name,       parent, compat, machine,            input,            class,                  init,       company,    fullname,                  flags
CONS( 2001, leappad,    0,      0,      leapfrog_leappad,   leapfrog_leappad, leapfrog_leappad_state, empty_init, "LeapFrog", "LeapPad",                 MACHINE_IS_SKELETON )
CONS( 2002, mfleappad,  0,      0,      leapfrog_mfleappad, leapfrog_leappad, leapfrog_leappad_state, empty_init, "LeapFrog", "My First LeapPad",        MACHINE_IS_SKELETON )
CONS( 2004, leappadmic, 0,      0,      leapfrog_leappad,   leapfrog_leappad, leapfrog_leappad_state, empty_init, "LeapFrog", "LeapPad Plus Microphone", MACHINE_IS_SKELETON ) // Compatible with regular LeapPad carts
CONS( 2005, ltleappad,  0,      0,      leapfrog_ltleappad, leapfrog_leappad, leapfrog_leappad_state, empty_init, "LeapFrog", "Little Touch LeapPad",    MACHINE_IS_SKELETON )
