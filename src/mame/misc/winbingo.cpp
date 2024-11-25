// license:BSD-3-Clause
// copyright-holders:

/*
'SH274 d' PCB by unknown manufacturer (probably Italian)
The only partially dumped game is Win Bingo 2006

Devices:
1x F3068F25V at u1 - 16-bit Single-Chip Microcomputer - main (not dumped)
1x SX28AC at u34 - Configurable Communications Controller (not dumped)
2x K6X1008C2D at u4 and u11 - RAMs
1x A42MX16 at u10 - Actel A42MX16 FPGA

OSCs:
1x oscillator 12.2880 MHz
1x oscillator 32768

Others:
1x 28x2 edge connector
1x 10 legs connector
1x 4 legs connector
1x RS232 connector
1x pushbutton (maybe an antitampering)
1x trimmer (volume)
*/

#include "emu.h"

#include "cpu/h8/h83048.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class winbingo_state : public driver_device
{
public:
	winbingo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void wbingo06(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


uint32_t winbingo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static INPUT_PORTS_START( wbingo06 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no dips on PCB
INPUT_PORTS_END


void winbingo_state::wbingo06(machine_config &config)
{
	H83048(config, "maincpu", 12.288_MHz_XTAL); // actually F3068F25V, H8/3068 core not available

	// SX28AC(config, "sx28ac", 32.768_kHz_XTAL); // clock for the internal RTC, chip should operate at higher frequencies

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(winbingo_state::screen_update));

	SPEAKER(config, "mono").front_center();
}


ROM_START( wbingo06 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "f3068f25v.u1", 0x0000, 0xc000, NO_DUMP ) // internal ROM

	ROM_REGION(0x4000, "sx28ac", 0)
	ROM_LOAD( "sx28ac.u34", 0x0000, 0x4000, NO_DUMP ) // internal EEPROM, wrong size

	ROM_REGION(0x380000, "unsorted", 0)
	ROM_LOAD( "m27c801.u12",  0x000000, 0x100000, CRC(00199a3c) SHA1(5d29dc0221d730dad53eecdf3a591098ab61da9c) )
	ROM_LOAD( "m27c801.u13",  0x100000, 0x100000, CRC(280e96aa) SHA1(98aced5a9afbf11a5aafd615b35ce8fd377f97bb) )
	ROM_LOAD( "m27c801.u14",  0x200000, 0x100000, CRC(5a8631fd) SHA1(9d22f41a9cf37eb74db53063aab8fae2006e29c9) )
	ROM_LOAD( "m29f040b.u21", 0x300000, 0x080000, NO_DUMP )

	ROM_REGION(0x2dd, "plds", 0)
	ROM_LOAD( "22cv10.u2", 0x000, 0x2dd, NO_DUMP )
ROM_END

} // Anonymous namespace


GAME( 2006, wbingo06, 0, wbingo06, wbingo06, winbingo_state, empty_init, ROT0, "<unknown>", "Win Bingo 2006", MACHINE_IS_SKELETON )
