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

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vreadere(machine_config &config);
};

uint32_t vreadere_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( vreadere )
INPUT_PORTS_END

MACHINE_CONFIG_START(vreadere_state::vreadere)
	/* basic machine hardware */
	// MCFG_DEVICE_ADD("maincpu", unknown, unknown) // CPU type is unknown, epoxy blob

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER( vreadere_state, screen_update )
MACHINE_CONFIG_END

ROM_START( vreadere )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD( "27-08291.u2", 0x000000, 0x400000, CRC(f2eb801f) SHA1(33e2d28ab2f04b17f66880898832265d50de54d4) )
ROM_END

COMP( 2004, vreadere, 0, 0, vreadere, vreadere, vreadere_state, empty_init, "Video Technology", "Reader Laptop E (Germany)", MACHINE_IS_SKELETON )
