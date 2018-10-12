// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

// Unknown CPU type

#include "emu.h"
#include "screen.h"

class glmmc_state : public driver_device
{
public:
	glmmc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void glmmc(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t glmmc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( glmmc )
INPUT_PORTS_END

MACHINE_CONFIG_START(glmmc_state::glmmc)
	/* basic machine hardware */
	// MCFG_DEVICE_ADD("maincpu", unknown, unknown) // CPU type is unknown, epoxy blob

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER( glmmc_state, screen_update )
MACHINE_CONFIG_END

ROM_START( glmmc )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-5889-00.bin", 0x000000, 0x080000, CRC(5e2c6359) SHA1(cc01c7bd5c87224b63dd1044db5a36a5cb7824f1) )
ROM_END

COMP( 19??, glmmc, 0, 0, glmmc, glmmc, glmmc_state, empty_init, "Video Technology", "Genius Leader Master Mega Color (Germany)", MACHINE_IS_SKELETON)
