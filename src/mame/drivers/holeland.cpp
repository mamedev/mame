// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Hole Land

    driver by Mathis Rosenhauer

    TODO:
    - tile/sprite priority in holeland (fixed? Needs further testing)
    - missing high bit of sprite X coordinate? (see round 2 and 3 of attract mode
      in crzrally)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "includes/holeland.h"


static ADDRESS_MAP_START( holeland_map, AS_PROGRAM, 8, holeland_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc001) AM_WRITE(holeland_pal_offs_w)
	AM_RANGE(0xc006, 0xc007) AM_WRITE(holeland_flipscreen_w)
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(holeland_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(holeland_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xf3ff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( crzrally_map, AS_PROGRAM, 8, holeland_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(holeland_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(holeland_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xebff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf000) AM_WRITE(holeland_scroll_w)
	AM_RANGE(0xf800, 0xf801) AM_WRITE(holeland_pal_offs_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, holeland_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(watchdog_reset_r)  /* ? */
	AM_RANGE(0x04, 0x04) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x06, 0x06) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x06, 0x07) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( holeland )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, "Nihongo" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPSETTING(    0x20, "90000" )
	PORT_DIPNAME( 0x40, 0x00, "Fase 3 Difficulty")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Mode" )
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Coin Case" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Monsters" )
	PORT_DIPSETTING(    0x40, "Min" )
	PORT_DIPSETTING(    0x00, "Max" )
	PORT_DIPNAME( 0x80, 0x80, "Mode" ) /* seems to have no effect */
	PORT_DIPSETTING(    0x00, "Stop" )
	PORT_DIPSETTING(    0x80, "Play" )
INPUT_PORTS_END

static INPUT_PORTS_START( crzrally )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Drive" )
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x04, "B" )
	PORT_DIPSETTING(    0x08, "C" )
	PORT_DIPSETTING(    0x0c, "D" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Extra Time" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "5 Sec" )
	PORT_DIPSETTING(    0x40, "10 Sec" )
	PORT_DIPSETTING(    0x60, "15 Sec" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controller ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Wheel" )
	PORT_DIPNAME( 0x80, 0x80, "Mode" ) /* seems to have no effect */
	PORT_DIPSETTING(    0x00, "Stop" )
	PORT_DIPSETTING(    0x80, "Play" )
INPUT_PORTS_END



static const gfx_layout holeland_charlayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0,0, 1,1, 2,2, 3,3, 8+0,8+0, 8+1,8+1, 8+2,8+2, 8+3,8+3 },
	{ 0*16,0*16, 1*16,1*16, 2*16,2*16, 3*16,3*16, 4*16,4*16, 5*16,5*16, 6*16,6*16, 7*16,7*16 },
	8*16
};

static const gfx_layout crzrally_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout holeland_spritelayout =
{
	32,32,
	RGN_FRAC(1,4),
	2,
	{ 4, 0 },
	{ 0, 2, 1, 3, 1*8+0, 1*8+2, 1*8+1, 1*8+3, 2*8+0, 2*8+2, 2*8+1, 2*8+3, 3*8+0, 3*8+2, 3*8+1, 3*8+3,
			4*8+0, 4*8+2, 4*8+1, 4*8+3, 5*8+0, 5*8+2, 5*8+1, 5*8+3, 6*8+0, 6*8+2, 6*8+1, 6*8+3, 7*8+0, 7*8+2, 7*8+1, 7*8+3 },
	{ 0, 4*64, RGN_FRAC(1,4), RGN_FRAC(1,4)+4*64, RGN_FRAC(2,4), RGN_FRAC(2,4)+4*64, RGN_FRAC(3,4), RGN_FRAC(3,4)+4*64,
		1*64, 5*64, RGN_FRAC(1,4)+1*64, RGN_FRAC(1,4)+5*64, RGN_FRAC(2,4)+1*64, RGN_FRAC(2,4)+5*64, RGN_FRAC(3,4)+1*64, RGN_FRAC(3,4)+5*64,
		2*64, 6*64, RGN_FRAC(1,4)+2*64, RGN_FRAC(1,4)+6*64, RGN_FRAC(2,4)+2*64, RGN_FRAC(2,4)+6*64, RGN_FRAC(3,4)+2*64, RGN_FRAC(3,4)+6*64,
		3*64, 7*64, RGN_FRAC(1,4)+3*64, RGN_FRAC(1,4)+7*64, RGN_FRAC(2,4)+3*64, RGN_FRAC(2,4)+7*64, RGN_FRAC(3,4)+3*64, RGN_FRAC(3,4)+7*64 },
	64*8
};

static const gfx_layout crzrally_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, 1 },
	{ 3*2, 2*2, 1*2, 0*2, 7*2, 6*2, 5*2, 4*2,
			16+3*2, 16+2*2, 16+1*2, 16+0*2, 16+7*2, 16+6*2, 16+5*2, 16+4*2 },
	{       RGN_FRAC(3,4)+0*16, RGN_FRAC(2,4)+0*16, RGN_FRAC(1,4)+0*16, RGN_FRAC(0,4)+0*16,
			RGN_FRAC(3,4)+2*16, RGN_FRAC(2,4)+2*16, RGN_FRAC(1,4)+2*16, RGN_FRAC(0,4)+2*16,
			RGN_FRAC(3,4)+4*16, RGN_FRAC(2,4)+4*16, RGN_FRAC(1,4)+4*16, RGN_FRAC(0,4)+4*16,
			RGN_FRAC(3,4)+6*16, RGN_FRAC(2,4)+6*16, RGN_FRAC(1,4)+6*16, RGN_FRAC(0,4)+6*16 },
	8*16
};

static GFXDECODE_START( holeland )
	GFXDECODE_ENTRY( "gfx1", 0, holeland_charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, holeland_spritelayout, 0, 256 )
GFXDECODE_END

static GFXDECODE_START( crzrally )
	GFXDECODE_ENTRY( "gfx1", 0, crzrally_charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, crzrally_spritelayout, 0, 256 )
GFXDECODE_END


static MACHINE_CONFIG_START( holeland, holeland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)        /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(holeland_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", holeland_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 2*16, 30*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(holeland_state, screen_update_holeland)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", holeland)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)
	MCFG_VIDEO_START_OVERRIDE(holeland_state,holeland)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1818182)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1818182)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/*

Crazy Rally
Tecfri, 1985

PCB Layout
|----------------------------------------------|
|                   20MHz                      |
|  Z80                                         |
|                                        PAL   |
|              3.7D      2149                  |
|  AY-3-8910                                   |
|  DSW2   DSW1 2.7F      2149         82S147.1F|
|1                                             |
|8 AY-3-8910   1.7G       5.5G                 |
|W           555                               |
|A                        4.5H                 |
|Y                                             |
|     VOL                 2128          9.1I   |
|         741                                  |
|    TDA1510                            8.1K   |
|              2149  2149  PAL                 |
|   82S129.9L  2149  2149  PAL          7.1L   |
|   82S129.9M  2149  2149  PAL                 |
|   82S129.9N  2149  2149               6.1N   |
|----------------------------------------------|
Notes:
      Z80 clock - 5.000MHz [20/4]
      AY3-8910 clock - 1.25MHz [20/16]
      VSync - 59Hz
      HSync - 15.08kHz

*/

static MACHINE_CONFIG_START( crzrally, holeland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 20000000/4)        /* 5 MHz */
	MCFG_CPU_PROGRAM_MAP(crzrally_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", holeland_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(holeland_state, screen_update_crzrally)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", crzrally)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)
	MCFG_VIDEO_START_OVERRIDE(holeland_state,crzrally)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 20000000/16)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 20000000/16)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( holeland )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "holeland.0",  0x0000, 0x2000, CRC(b640e12b) SHA1(68d091a92747d2f4534386aff3ddb07c0d79384c) )
	ROM_LOAD( "holeland.1",  0x2000, 0x2000, CRC(2f180851) SHA1(c21bcd3e9ff31a5cc415eb53d77a9cc9ebdd862d) )
	ROM_LOAD( "holeland.2",  0x4000, 0x2000, CRC(35cfde75) SHA1(0a03c0464c771d049ae8706793ec43da5372fa58) )
	ROM_LOAD( "holeland.3",  0x6000, 0x2000, CRC(5537c22e) SHA1(030f34d3cbc5eea30a3ede77008eba394ef37e8f) )
	ROM_LOAD( "holeland.4",  0xa000, 0x2000, CRC(c95c355d) SHA1(44984108b6a3dab05855da4c4a3ff58d849559b8) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "holeland.5",  0x0000, 0x2000, CRC(7f19e1f9) SHA1(75026da91e0cff262e5f6e32f836907a786aef42) )
	ROM_LOAD( "holeland.6",  0x2000, 0x2000, CRC(844400e3) SHA1(d306b26f838b043b71c5f9d2d240228986b695fa) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "holeland.7",  0x0000, 0x2000, CRC(d7feb25b) SHA1(581e20b07d33ba350601fc56074c43aaf13078b4) )
	ROM_LOAD( "holeland.8",  0x2000, 0x2000, CRC(4b6eec16) SHA1(4c5da89c2babeb33951d101703e6699fbcb886b4) )
	ROM_LOAD( "holeland.9",  0x4000, 0x2000, CRC(6fe7fcc0) SHA1(fa982551285f728cee0055a0c473f6c74d802d2e) )
	ROM_LOAD( "holeland.10", 0x6000, 0x2000, CRC(e1e11e8f) SHA1(56082fe497d8ee8ecfe1b89c0c5ada4ddfa4740f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "3m",          0x0000, 0x0100, CRC(9d6fef5a) SHA1(e2b62909fecadfc9e0eb1ad72c8b7712a26d184e) )  /* Red component */
	ROM_LOAD( "3l",          0x0100, 0x0100, CRC(f6682705) SHA1(1ab952c1e2a45e9b0dc9144f50711f99f6b1ebc4) )  /* Green component */
	ROM_LOAD( "3n",          0x0200, 0x0100, CRC(3d7b3af6) SHA1(0c4f95b26e9fe25a5d8c79f06e7ceab78a07d35c) )  /* Blue component */
ROM_END


ROM_START( crzrally )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7g",        0x0000, 0x4000, CRC(8fe01f86) SHA1(3e08f2cdcd08b25f2bb32d1c4d4caf4ac60c94d6) )
	ROM_LOAD( "2.7f",        0x4000, 0x4000, CRC(67110f1d) SHA1(cc500017057e39cc8a6cb4e4ccae3c3cbab6c2ba) )
	ROM_LOAD( "3.7d",        0x8000, 0x4000, CRC(25c861c3) SHA1(cc9f5f33833279b4430a4b8497cc16a222d31805) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",        0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "5.5g",        0x2000, 0x2000, CRC(b34aa904) SHA1(fb4301fd06efc33df9d9f611c3e67a9f7198531d) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "6.1f",        0x0000, 0x2000, CRC(a909ff0f) SHA1(9ce37a6bbb09c936551082dea62a791d10d7d346) )
	ROM_LOAD( "7.1l",        0x2000, 0x2000, CRC(38fb0a16) SHA1(a17ec5c9acc5c244ffc715ee2376fbf8209e72fd) )
	ROM_LOAD( "8.1k",        0x4000, 0x2000, CRC(660aa0f0) SHA1(1bb85851349f772f21db9629b0086b2460614b9d) )
	ROM_LOAD( "9.1i",        0x6000, 0x2000, CRC(37d0790e) SHA1(877335a06d1842264daff9eb46d6ea1ce8249c29) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  /* Red component */
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  /* Green component */
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  /* Blue component */

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k",  0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r4a.5m",  0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r8a.1d",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( crzrallya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crzralla_1.7g",        0x0000, 0x4000, CRC(8c6a70aa) SHA1(61b10cb16ddce813a768181483b03bead5b05702) )
	ROM_LOAD( "crzralla_2.7f",        0x4000, 0x4000, CRC(7fdd4a45) SHA1(194d504adfd83adc52df2df27a18116a3072ea9d) )
	ROM_LOAD( "crzralla_3.7d",        0x8000, 0x4000, CRC(a25edd17) SHA1(8f883bf3e42b9bf929717f6f13a281f0b83669b1) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",                 0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "crzralla_5.5g",        0x2000, 0x2000, CRC(81e9b043) SHA1(effc082a025ce36ab6ba8603a82be1469eee6276) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "6.1f",        0x0000, 0x2000, CRC(a909ff0f) SHA1(9ce37a6bbb09c936551082dea62a791d10d7d346) )
	ROM_LOAD( "7.1l",        0x2000, 0x2000, CRC(38fb0a16) SHA1(a17ec5c9acc5c244ffc715ee2376fbf8209e72fd) )
	ROM_LOAD( "8.1k",        0x4000, 0x2000, CRC(660aa0f0) SHA1(1bb85851349f772f21db9629b0086b2460614b9d) )
	ROM_LOAD( "9.1i",        0x6000, 0x2000, CRC(37d0790e) SHA1(877335a06d1842264daff9eb46d6ea1ce8249c29) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  /* Red component */
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  /* Green component */
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  /* Blue component */

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k",  0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r4a.5m",  0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r8a.1d",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( crzrallyg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12.7g",       0x0000, 0x4000, CRC(0cab3ef9) SHA1(6de4d4a7159e0a6ad13dbca3344759410618ea26) )
	ROM_LOAD( "13.7f",       0x4000, 0x4000, CRC(e19a8e13) SHA1(1462b21f16990eb9ae2f2d1cd5c097edf88bf614) )
	ROM_LOAD( "14.7d",       0x8000, 0x4000, CRC(4c0351ba) SHA1(0ed04825d3affe0477bb963f1c96ff223e4bcf50) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",        0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "16.5g",       0x2000, 0x2000, CRC(94289f9e) SHA1(8da00814d8f769de124bc09f4c1ee851c99cec0e) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "17.1n",       0x0000, 0x2000, CRC(985ed5c8) SHA1(ee91a6701a8b8bb24d6fa08596deff95816e759e) )
	ROM_LOAD( "18.1l",       0x2000, 0x2000, CRC(c02ddda2) SHA1(262e33cada0e7935d03014583117c2bc6278865b) )
	ROM_LOAD( "19.1k",       0x4000, 0x2000, CRC(2a0d5bca) SHA1(8d7aedd63ea374a5809c24f957b0afa3cad437d0) )
	ROM_LOAD( "20.1i",       0x6000, 0x2000, CRC(49c0c2b8) SHA1(30c4fe1dc2df499927f8fd4a041a707b81a04e1d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  /* Red component */
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  /* Green component */
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  /* Blue component */

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END


GAME( 1984, holeland,  0,        holeland, holeland, driver_device, 0, ROT0,   "Tecfri", "Hole Land",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrally,  0,        crzrally, crzrally, driver_device, 0, ROT270, "Tecfri", "Crazy Rally (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrallya, crzrally, crzrally, crzrally, driver_device, 0, ROT270, "Tecfri", "Crazy Rally (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrallyg, crzrally, crzrally, crzrally, driver_device, 0, ROT270, "Tecfri (Gecas license)", "Crazy Rally (Gecas license)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
