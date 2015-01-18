#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/banctec.h"

static ADDRESS_MAP_START( banctec_mem , AS_PROGRAM, 8, banctec_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

#if 0
static const gfx_layout banctec_charlayout =
{
		8,8,
		256*8,                                    /* 256 characters */
		1,                      /* 1 bits per pixel */
		{ 0 },                  /* no bitplanes; 1 bit per pixel */
		/* x offsets */
		{
			0,
			1,
			2,
			3,
			4,
			5,
			6,
			7,
		},
		/* y offsets */
		{
			0,
			8,
			16,
			24,
			32,
			40,
			48,
			56,
		},
		8*8
};

static GFXDECODE_START( banctec )
	GFXDECODE_ENTRY( "gfx1", 0x0000, banctec_charlayout, 0, 2 )
GFXDECODE_END
#endif

void banctec_state::machine_reset()
{
}

static MACHINE_CONFIG_START( banctec, banctec_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6805, 4000000)     /* 4000000? */
	MCFG_CPU_PROGRAM_MAP(banctec_mem)

#if 0
	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(LCD_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*4, 128) /* 160 x 102 */
	MCFG_SCREEN_VISIBLE_AREA(0, 64*4-1, 0, 128-1)
	MCFG_SCREEN_UPDATE_DRIVER(banctec_state, screen_update_banctec)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", banctec )
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
#endif

MACHINE_CONFIG_END

ROM_START(banctec)
	ROM_REGION(0x2000,"maincpu",0)
	ROM_LOAD("banctec_eseries_panel.u8", 0x0000, 0x2000, CRC(f3335e0a) SHA1(5ca45fdcb7ef45a65c28c79abfa9ebb7a8a06619))

	ROM_REGION(0x1000,"gfx",0)
	ROM_LOAD("banctec_eseries_panel.u20", 0x0000, 0x1000, CRC(5b6ecec9) SHA1(35aff8f965bce77205e3a43d71e39097585091a7))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT    MONITOR COMPANY   FULLNAME */
CONS( 1993, banctec, 0,        0,      banctec, 0, driver_device, 0,       "BancTec",  "ESeries Panel", GAME_NOT_WORKING | GAME_NO_SOUND)
