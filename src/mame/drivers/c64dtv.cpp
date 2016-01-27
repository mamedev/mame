// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"


//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class c64dtv_state : public driver_device
{
public:
	// constructor
	c64dtv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black);
		return 0;
	}
};



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( c64dtv )
INPUT_PORTS_END



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( c64dtv, c64dtv_state )
	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(c64dtv_state, screen_update)
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MCFG_SCREEN_REFRESH_RATE(30)
MACHINE_CONFIG_END



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



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

CONS( 2005, c64dtv,  0,  0,    c64dtv,     c64dtv, driver_device,     0,     "The Toy:Lobster Company", "Commodore 64 Direct-to-TV (Version 2 050711) (PAL)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
