/******************************************************************************

  San Remo Games.
  8bit gambling hardware.

  Driver by Roberto Fresca.


*******************************************************************************

  *** Hardware notes ***

  - CPU:  1x TMPZ84C00AP-6.
  - CRTC: 1x MC6845.
  - SND:  1x WF19054 (AY-3-8910).
  - CLK:  1x crystal @ 18.000 MHz.

  - PLDs: 3x PALCE (read protected).


*******************************************************************************

  Game Notes
  ----------

  Nothing yet...


*******************************************************************************

  Driver updates:

  [2012/01/19]    Preliminary driver...


  TODO:

  - Fix video RAM
  - Interrupts.
  - Hook inputs.
  - Hook AY-8910.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_18MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


class sanremo_state : public driver_device
{
public:
	sanremo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(sanremo_videoram_w);
	DECLARE_WRITE8_MEMBER(sanremo_colorram_w);
	TILE_GET_INFO_MEMBER(get_sanremo_tile_info);
	DECLARE_READ8_MEMBER(testa_r);
	DECLARE_WRITE8_MEMBER(testa_w);
	virtual void video_start();
	UINT32 screen_update_sanremo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*************************
*     Video Hardware     *
*************************/


WRITE8_MEMBER(sanremo_state::sanremo_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanremo_state::sanremo_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(sanremo_state::get_sanremo_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER( 0, code, attr, 0);

}

void sanremo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(sanremo_state::get_sanremo_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 sanremo_state::screen_update_sanremo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}



/******************************
*         R/W Handlers        *
******************************/

READ8_MEMBER(sanremo_state::testa_r)
{
	return machine().rand() & 0xff;
}

WRITE8_MEMBER(sanremo_state::testa_w)
{
//	printf("%02x TESTA\n", data);
}



/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( sanremo_map, AS_PROGRAM, 8, sanremo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM // seems to be working RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(sanremo_videoram_w) AM_SHARE("videoram")	// filling 8000-87ff with 0x0b ???
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(sanremo_colorram_w) AM_SHARE("colorram")	// filling 8000-87ff with 0x0b ???
	AM_RANGE(0xf800, 0xffff) AM_RAM // dunno.. writting at very end.

ADDRESS_MAP_END

static ADDRESS_MAP_START( sanremo_portmap, AS_IO, 8, sanremo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(testa_r)
	AM_RANGE(0x03, 0x03) AM_WRITE(testa_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x14, 0x14) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)

/*

  00 W
  01 R -- read and xor to 03
  02 R
  03 W
  04 W -- CRTC address
  06 W
  14 W -- CRTC register
  15 W
  17 W
  24 W -- sequence 05 05 05 05 05 05 05 05 05 06 07 0A 0B 0C 0D
  27 R
  37 W

*/

ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( sanremo )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( sanremo )
	GFXDECODE_ENTRY( "gfxbnk1", 0, tilelayout, 0, 1 )
	GFXDECODE_ENTRY( "gfxbnk0", 0, charlayout, 0, 8 )
GFXDECODE_END


/************************
*    CRTC Interface    *
************************/

static MC6845_INTERFACE( mc6845_intf )
{
	"screen",   /* screen we are acting on */
	false,		/* show border area */
	8,          /* number of pixels per video memory address */
	NULL,       /* before pixel update callback */
	NULL,       /* row update callback */
	NULL,       /* after pixel update callback */
	DEVCB_NULL, /* callback for display state changes */
	DEVCB_NULL, /* callback for cursor state changes */
	DEVCB_NULL, /* HSYNC callback */
	DEVCB_NULL, /* VSYNC callback */
	NULL        /* update address callback */
};


/*************************
*    Sound Interfaces    *
*************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( sanremo, sanremo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(sanremo_map)
	MCFG_CPU_IO_MAP(sanremo_portmap)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", sanremo_state,  nmi_line_pulse)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(70*8, 41*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 48*8-1, 0, 38*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sanremo_state, screen_update_sanremo)

	MCFG_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/12, mc6845_intf)

	MCFG_GFXDECODE(sanremo)
	MCFG_PALETTE_LENGTH(0x200)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/12)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( sanremo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "no_g0.ic26",	0x0000, 0x8000, CRC(2d83646f) SHA1(d1fafcce44ed3ec3dd53d84338c42244ebfca820) )

	ROM_REGION( 0x10000, "gfxbnk0", 0 )
	ROM_LOAD( "no_i4.ic30",	0x00000, 0x10000, CRC(55b351a4) SHA1(b0c8a30dde076520234281da051f21f1b7cb3166) )	// i?

	ROM_REGION( 0x30000, "gfxbnk1", 0 )
	ROM_LOAD( "no_b4.ic27",	0x00000, 0x10000, CRC(e48b1c8a) SHA1(88f60268fd43c06e146d936a1bdc078c44e2a213) )	// b
	ROM_LOAD( "no_g4.ic28",	0x10000, 0x10000, CRC(4eea9a9b) SHA1(c86c083ccf08c3c310028920f9a0fe809fd7ccbe) )	// g
	ROM_LOAD( "no_r4.ic29",	0x20000, 0x10000, CRC(ab08cdaf) SHA1(e0518403039b6bada79ffe4c6bc22fbb64d16e43) )	// r

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce1.bin",	0x0000, 0x0104, NO_DUMP )	/* PALCE is read protected */
	ROM_LOAD( "palce2.bin",	0x0200, 0x0104, NO_DUMP )	/* PALCE is read protected */
	ROM_LOAD( "palce3.bin",	0x0400, 0x0104, NO_DUMP )	/* PALCE is read protected */
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME     PARENT  MACHINE  INPUT    STATE          INIT   ROT    COMPANY           FULLNAME                      FLAGS... */
GAME( 198?, sanremo, 0,      sanremo, sanremo, driver_device, 0,     ROT0, "San Remo Games", "Unknown San Remo poker game", GAME_NOT_WORKING )
