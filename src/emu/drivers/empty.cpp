// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    empty.cpp

    Empty driver.

**************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "screen.h"

//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class empty_state : public driver_device
{
public:
	// constructor
	using driver_device::driver_device;

	void ___empty(machine_config &config);

	virtual std::vector<std::string> searchpath() const override { return std::vector<std::string>(); }

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

void empty_state::___empty(machine_config &config)
{
	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(empty_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_refresh_hz(30);
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ___empty )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2007, ___empty, 0, ___empty, 0, empty_state, empty_init, ROT0, "MAME", "No Driver Loaded", MACHINE_NO_SOUND_HW )
