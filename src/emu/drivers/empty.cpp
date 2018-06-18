// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    empty.c

    Empty driver.

**************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "render.h"

//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class empty_state : public driver_device
{
public:
	// constructor
	using driver_device::driver_device;

	void ___empty(machine_config &config);

protected:
	virtual void machine_start() override
	{
		emulator_info::display_ui_chooser(machine());
	}

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}
};



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

MACHINE_CONFIG_START( empty_state::___empty )

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(empty_state, screen_update)
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MCFG_SCREEN_REFRESH_RATE(30)
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ___empty )
	ROM_REGION( 0x10, "user1", ROMREGION_ERASEFF )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2007, ___empty, 0, ___empty, 0, empty_state, empty_init, ROT0, "MAME", "No Driver Loaded", MACHINE_NO_SOUND_HW )
