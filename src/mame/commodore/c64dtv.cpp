// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*************************************************************************************************************

Commodore 64 Direct-to-TV
It looks like a fat joystick.

Chips: All unmarked. According to Wikipedia an ASIC contains an entire
       Commodore 64 (6510 CPU, VIC-II, SID, CIA, PLA) running at 32MHz. The
       6510 CPU portion runs at 1MHz.
Crystals: 32.720 (X1), unmarked (X2).

*************************************************************************************************************/
#include "emu.h"
#include "screen.h"


namespace {

//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class c64dtv_state : public driver_device
{
public:
	// constructor
	c64dtv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}
	void c64dtv(machine_config &config);
};



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( c64dtv )
INPUT_PORTS_END



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void c64dtv_state::c64dtv(machine_config &config)
{
	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(c64dtv_state::screen_update));
	screen.set_size(640,480);
	screen.set_visarea_full();
	screen.set_refresh_hz(30);
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// BASIC sits at 0xa000-0xc000, chargen-like chunks sit at 0x1000-0x2000, 0x9000-0xa000 and 0xd000-0xe000
// kernel sits at 0xe000
// from 0x10000 on there are the games
ROM_START( c64dtv )
	ROM_REGION( 0x200000, "asic", 0 )
	ROM_LOAD( "flash.u2", 0x000000, 0x200000, CRC(b820375a) SHA1(b9f88919e2bed825eb2b2cb605977d55971b423b) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

CONS( 2005, c64dtv, 0, 0, c64dtv, c64dtv, c64dtv_state, empty_init, "The Toy:Lobster Company", "Commodore 64 Direct-to-TV (Version 2 050711) (PAL)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
