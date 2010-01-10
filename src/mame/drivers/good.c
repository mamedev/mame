/*
  'Good' Driver by David Haywood
  todo: finish inputs etc.


+----------------------------+
|    voice.rom  6116         |
|      M6295    6116         |
|J             HY6264        |
|A             HY6264 grp-01 |
|M DSW1 DSW2 A40MX04  grp-02 |
|M                    grp-03 |
|A IS61C256x2         grp-04 |
| s1 s2                      |
| 68000      16MHz 12MHz     |
+----------------------------+

Motorola MC68000P8
OKI M6295 (badged as AD-65)
Actel A40MX04-F PL84

Silk screened under the roms:

  system1 - SYSTEM 1
  system2 - SYSTEM 2
   grp-01 - GRP-01
   grp-02 - GRP-02
   grp-03 - GRP-03
   grp-04 - GRP-04
voice.rom - VOICE ROM
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


typedef struct _good_state good_state;
struct _good_state
{
	/* memory pointers */
	UINT16 *  bg_tilemapram;
	UINT16 *  fg_tilemapram;
	UINT16 *  sprites;
//  UINT16 *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *bg_tilemap,*fg_tilemap;
};


static WRITE16_HANDLER( fg_tilemapram_w )
{
	good_state *state = (good_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fg_tilemapram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	good_state *state = (good_state *)machine->driver_data;
	int tileno = state->fg_tilemapram[tile_index * 2];
	int attr = state->fg_tilemapram[tile_index * 2 + 1] & 0xf;
	SET_TILE_INFO(0, tileno, attr, 0);
}

static WRITE16_HANDLER( bg_tilemapram_w )
{
	good_state *state = (good_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg_tilemapram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	good_state *state = (good_state *)machine->driver_data;
	int tileno = state->bg_tilemapram[tile_index * 2];
	int attr = state->bg_tilemapram[tile_index * 2 + 1] & 0xf;
	SET_TILE_INFO(1, tileno, attr, 0);
}



static VIDEO_START( good )
{
	good_state *state = (good_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0xf);
}

static VIDEO_UPDATE( good )
{
	good_state *state = (good_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( good_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM

	//AM_RANGE(0x270000, 0x270007) AM_RAM // scroll?
	AM_RANGE(0x270000, 0x270001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)

	AM_RANGE(0x280000, 0x280001) AM_READ_PORT("IN0")
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN1")
	AM_RANGE(0x280004, 0x280005) AM_READ_PORT("IN2")

	AM_RANGE(0x800000, 0x8007ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x820000, 0x820fff) AM_RAM_WRITE(fg_tilemapram_w) AM_BASE_MEMBER(good_state, fg_tilemapram)
	AM_RANGE(0x822000, 0x822fff) AM_RAM_WRITE(bg_tilemapram_w) AM_BASE_MEMBER(good_state, bg_tilemapram)

	AM_RANGE(0xff0000, 0xffefff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( good )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "1" )
	PORT_DIPSETTING(  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )

	/*The following appears to be DSW*/
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, "Credits per Coin" )
	PORT_DIPSETTING(  0x000e, "50"  )
	PORT_DIPSETTING(  0x000c, "60"  )
	PORT_DIPSETTING(  0x000a, "70"  )
	PORT_DIPSETTING(  0x0008, "80"  )
	PORT_DIPSETTING(  0x0006, "100" )
	PORT_DIPSETTING(  0x0004, "120" )
	PORT_DIPSETTING(  0x0002, "150" )
	PORT_DIPSETTING(  0x0000, "200" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Double Up Test Mode" )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Mature Content" )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout good_layout2 =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0,1,2,3,4,5,6,7, 256,257,258,259,260,261,262,263 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16  },
	32*16
};


static GFXDECODE_START( good )
	GFXDECODE_ENTRY( "gfx1", 0, good_layout2,  0x100, 16  ) /* fg tiles */
	GFXDECODE_ENTRY( "gfx1", 0, good_layout2,  0x200, 16  ) /* fg tiles */
GFXDECODE_END


static MACHINE_DRIVER_START( good )
	MDRV_DRIVER_DATA(good_state)

	MDRV_CPU_ADD("maincpu", M68000, 16000000 /2)
	MDRV_CPU_PROGRAM_MAP(good_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_GFXDECODE(good)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(1*16, 23*16-1, 0*16, 14*16-1)

	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(good)
	MDRV_VIDEO_UPDATE(good)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("oki", OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_DRIVER_END


ROM_START( good )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "system1", 0x00001, 0x10000, CRC(128374cb) SHA1(a6521f506a6e4f8e62936ec0e66b080106a43f36) )
	ROM_LOAD16_BYTE( "system2", 0x00000, 0x10000, CRC(c4eada4e) SHA1(2d9875487626796db8633520625ad6ad642723ef) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "voice.rom", 0x00000, 0x40000, CRC(a5a23482) SHA1(51ca69589086c1da44d64575ee9a4da7b88ba669) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "grp-01", 0x00000, 0x20000, CRC(33f8458e) SHA1(7f0c5fb44e3350c579f2dea82f0ec1d2ac5967ff) )
	ROM_LOAD16_BYTE( "grp-02", 0x00001, 0x20000, CRC(c0b98e6c) SHA1(fdafced2418feeec0ed3d87bbbc88d5aa28f380a) )
	ROM_LOAD16_BYTE( "grp-03", 0x40000, 0x20000, CRC(41da3bf4) SHA1(e7d1973951d4470fd2e0fa3c4690633219b71c06) )
	ROM_LOAD16_BYTE( "grp-04", 0x40001, 0x20000, CRC(83dbbb52) SHA1(e597f3cbb54b5cdf2230ea6318f970319061e31b) )
ROM_END

GAME( 1998, good,   0,   good,   good,   0,  ROT0,  "<unknown>", "Good (Korea)", GAME_SUPPORTS_SAVE )
