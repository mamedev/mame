// license:BSD-3-Clause
// copyright-holders:

/*
Chaku Melo Collection
Capcom 99705-01 PCB
Cellphone ring-tone vending machine.


Main components:
Capcom DL-3229 SCU (encryption unused)
6.14SC9K XTAL (near SCU)
4x 5264805DLTTA60 PC100-222-60 RAM (near SCU)
2x PC16552DV DUART
18.4SC9K XTAL (near PC16552DV 1)
18.4SC9K XTAL (near PC16552DV 2)
HD64412F GFX chip
33.0000 MHz XTAL (near HD64412F)
2x M5118165D-60J RAM (near HD64412F)
MC44200FT Triple 8-bit video DAC
2x SP232ACN RS-232 Line Drivers/Receivers
12.5984 MHz XTAL (near SP232ACN and DAC)
M4T28-BR12SH1 timekeeper (marked M48T58Y on PCB)
XC9536 CPLD
YMZ705-F sound chip
2x HM62W8512BLTTI7 RAM (near YMZ705-F)
6.14SC9K XTAL (near YMZ705-F)

TODO: everything (only BIOS is dumped, needs HD dump)
*/


#include "emu.h"

#include "cpu/sh/sh7604.h"
#include "machine/ins8250.h"
#include "machine/timekpr.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class chakumelo_state : public driver_device
{
public:
	chakumelo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void chakumel(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void chakumelo_state::program_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
}


static INPUT_PORTS_START( chakumel )
INPUT_PORTS_END


void chakumelo_state::chakumel(machine_config &config)
{
	SH7604(config, m_maincpu, 6'140'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &chakumelo_state::program_map);

	PC16552D(config, "duart0", 0);

	PC16552D(config, "duart1", 0);

	// HD64412F(config, "hd64412f", 33_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	// YMZ705-F(config, "ymz", 6'140'000);
}


ROM_START( chakumel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "cmcja.6c", 0x00000, 0x80000, CRC(becd3703) SHA1(41a4e512ee6129029161d342fb46351a0737822b) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 ) // all PALCE16V8H
	ROM_LOAD( "cmc4b.4b", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "cmc4c.4c", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "cmc5c.5c", 0x400, 0x117, NO_DUMP )

	DISK_REGION( "hdd" )
	DISK_IMAGE( "chakumel", 0, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1999, chakumel, 0, chakumel, chakumel, chakumelo_state, empty_init, ROT0, "Capcom", "Chaku Melo Collection", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
