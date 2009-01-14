/*******************************************************************************************

Casino Winner (c) 1985 Aristocrat

driver by Chris Hardy & Angelo Salese

TODO:
-missing prom;
-NVRAM emulation? Likely to be just backup ram
-Cherry-type subgames appears to have wrong graphics alignment,maybe it's some fancy window
 effect?

*******************************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

static UINT8 *sc0_vram,*sc0_attr;
static tilemap *sc0_tilemap;

static TILE_GET_INFO( get_sc0_tile_info )
{
	int tile = (sc0_vram[tile_index] | ((sc0_attr[tile_index] & 0x70)<<4)) & 0x7ff;
	int colour = sc0_attr[tile_index] & 0xf;

	SET_TILE_INFO(
			0,
			tile,
			colour,
			0);
}

static VIDEO_START(vvillage)
{
	sc0_tilemap = tilemap_create(machine, get_sc0_tile_info,tilemap_scan_rows,8,8,32,32);
}

static VIDEO_UPDATE(vvillage)
{
	tilemap_draw(bitmap,cliprect,sc0_tilemap,0,0);
	return 0;
}

static WRITE8_HANDLER( sc0_vram_w )
{
	sc0_vram[offset] = data;
	tilemap_mark_tile_dirty(sc0_tilemap,offset);
}

static WRITE8_HANDLER( sc0_attr_w )
{
	sc0_attr[offset] = data;
	tilemap_mark_tile_dirty(sc0_tilemap,offset);
}

/*These two are tested during the two cherry sub-games.I really don't know what is supposed to do...*/
static WRITE8_HANDLER( scroll_w )
{
}

/*---- --x- enable some weird effect*/
static WRITE8_HANDLER( vregs_w )
{
}

/**********************
*
* End of Video Hardware
*
**********************/

static READ8_HANDLER( input_buttons_r )
{
	return input_port_read(space->machine,"DSW1");
}

static READ8_HANDLER( input_coins_r )
{
	return input_port_read(space->machine,"DSW2");
}

static READ8_HANDLER( vvillage_rng_r )
{
	return mame_rand(space->machine);
}

static WRITE8_HANDLER( output_w )
{
	coin_counter_w(0,data & 1);
	coin_counter_w(1,data & 1);
	coin_lockout_w(0,data & 0x20);
	coin_lockout_w(1,data & 0x20);
}

static WRITE8_HANDLER( lamps_w )
{
	/*
    ---x ---- lamp button 5
    ---- x--- lamp button 4
    ---- -x-- lamp button 3
    ---- --x- lamp button 2
    ---- ---x lamp button 1
    */
}

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ(vvillage_rng_r)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(sc0_vram_w) AM_BASE(&sc0_vram)
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(sc0_attr_w) AM_BASE(&sc0_attr)
ADDRESS_MAP_END

static ADDRESS_MAP_START( portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0x02,0x02) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x03,0x03) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11,0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x10,0x10) AM_WRITE(scroll_w)
	AM_RANGE(0x11,0x11) AM_WRITE(vregs_w)
	AM_RANGE(0x12,0x12) AM_WRITE(lamps_w)
	AM_RANGE(0x13,0x13) AM_WRITE(output_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( vvillage )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Poker" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Black Jack" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Hi & Low" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Five Line" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Super Conti" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hopper" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Show Dip-Switches Status" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) ) //analyzer
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0,RGN_FRAC(1,2) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( vvillage )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_buttons_r,
	input_coins_r,
	NULL,
	NULL
};

static MACHINE_DRIVER_START( vvillage )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,0)
	MDRV_CPU_IO_MAP(portmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(vvillage)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(vvillage)
	MDRV_VIDEO_UPDATE(vvillage)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 4000000 / 4)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END

ROM_START( caswin )
	ROM_REGION( 0x8000, "main", 0 )
	ROM_LOAD( "cw_v5_0_1.26", 0x0000, 0x4000, CRC(ae3d2cf0) SHA1(268572730389f12cf962782008690305fad1ac1b) )
	ROM_LOAD( "cw_v5_0_2.24", 0x4000, 0x4000, CRC(2855b3b8) SHA1(f5cc0bbeee6c1fb0dc6aebc2e3af09dccdb248ad) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "cw_4.19", 0x00000, 0x4000, CRC(d2deab75) SHA1(12cf3fd02dbad9a40cfa6cece0cb66ce2c4dc315) )
	ROM_LOAD( "cw_3.22", 0x04000, 0x4000, CRC(7e79966c) SHA1(39190ee8cd7f3b8f895b32327f3a5555a0713315) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom.x", 0x00, 0x20, NO_DUMP )
ROM_END


GAME( 1985, caswin, 0, vvillage, vvillage, 0, ROT270, "Aristocrat",  "Casino Winner", GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS )
