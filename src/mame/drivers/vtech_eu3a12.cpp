// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

// CPU die is an Elan EU3A12, it isn't clear what it is compatible with (16-bit?)

#include "emu.h"
#include "screen.h"

class vreadere_state : public driver_device
{
public:
	vreadere_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void vreadere(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t vreadere_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( vreadere )
INPUT_PORTS_END

void vreadere_state::vreadere(machine_config &config)
{
	// UNKNOWN(config, "maincpu", unknown); // CPU type is unknown, epoxy blob

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(vreadere_state::screen_update));
}

ROM_START( vreadere )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD( "27-08291.u2", 0x000000, 0x400000, CRC(f2eb801f) SHA1(33e2d28ab2f04b17f66880898832265d50de54d4) )
ROM_END

COMP( 2004, vreadere, 0, 0, vreadere, vreadere, vreadere_state, empty_init, "Video Technology", "Reader Laptop E (Germany)", MACHINE_IS_SKELETON )
