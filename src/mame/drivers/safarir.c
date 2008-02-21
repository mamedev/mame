/****************************************************************************

    Safari Rally hardware

    driver by Zsolt Vasvari

    Games Supported:
        * Safari Rally (Japan)

    Known issues/to-do's:
        * SN76477 sound
        * Background colors are wrong.
          Flyer shows red and perhaps blue,
          but I don't see an obvious mapping

    Notes:
        * This hardware is a precursor to Phoenix

          ----------------------------------

          CPU board

          76477        18MHz

                        8080

          ----------------------------------

          Video board


           RL07  2114
                 2114
                 2114
                 2114
                 2114           RL01 RL02
                 2114           RL03 RL04
                 2114           RL05 RL06
           RL08  2114

           11MHz

          ----------------------------------

****************************************************************************/

#include "driver.h"


static UINT8 *ram_1, *ram_2;
static size_t ram_size;
static UINT8 ram_bank;
static tilemap *bg_tilemap, *fg_tilemap;
static UINT8 *bg_scroll;



/*************************************
 *
 *  RAM banking
 *
 *************************************/

static WRITE8_HANDLER( ram_w )
{
	if (ram_bank)
		ram_2[offset] = data;
	else
		ram_1[offset] = data;

	tilemap_mark_tile_dirty((offset & 0x0400) ? bg_tilemap : fg_tilemap, offset & 0x03ff);
}


static READ8_HANDLER( ram_r )
{
	return ram_bank ? ram_2[offset] : ram_1[offset];
}


static WRITE8_HANDLER( ram_bank_w )
{
	ram_bank = data & 0x01;

	tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 chars */
	128,	/* 128 characters */
	1,		/* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( safarir )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout, 0, 8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, charlayout, 0, 8 )
GFXDECODE_END


static PALETTE_INIT( safarir )
{
	int i;

	for (i = 0; i < machine->config->total_colors / 2; i++)
	{
		palette_set_color(machine, (i * 2) + 0, RGB_BLACK);
		palette_set_color(machine, (i * 2) + 1, MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0)));
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int color;
	UINT8 code = ram_r(tile_index | 0x400);

	/* this is wrong */
	if (code & 0x80)
		color = 6;	/* yellow */
	else
		color = 2;  /* green */

	SET_TILE_INFO(0, code & 0x7f, color, 0);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	int color, flags;
	UINT8 code = ram_r(tile_index);

	if (code & 0x80)
		color = 7;	/* white */
	else
		color = (~tile_index & 0x04) | ((tile_index >> 1) & 0x03);

	flags = ((tile_index & 0x1f) >= 0x03) ? 0 : TILE_FORCE_LAYER0;

	SET_TILE_INFO(1, code & 0x7f, color, flags);
}


static VIDEO_START( safarir )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}


static VIDEO_UPDATE( safarir )
{
	tilemap_set_scrollx(bg_tilemap, 0, *bg_scroll);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}



/*************************************
 *
 *  Machine start
 *
 *************************************/

static MACHINE_START( safarir )
{
	ram_1 = auto_malloc(ram_size);
	ram_2 = auto_malloc(ram_size);

	/* setup for save states */
	state_save_register_global_pointer(ram_1, ram_size);
	state_save_register_global_pointer(ram_2, ram_size);
	state_save_register_global(ram_bank);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(ram_r, ram_w) AM_SIZE(&ram_size)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_READWRITE(MRA8_NOP, ram_bank_w)
	AM_RANGE(0x2c00, 0x2cff) AM_MIRROR(0x03ff) AM_READWRITE(MRA8_NOP, MWA8_RAM) AM_BASE(&bg_scroll)
	AM_RANGE(0x3000, 0x30ff) AM_MIRROR(0x03ff) AM_WRITENOP	/* goes to SN76477 */
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_WRITENOP 	/* cleared at the beginning */
	AM_RANGE(0x3800, 0x38ff) AM_MIRROR(0x03ff) AM_READWRITE(input_port_0_r, MWA8_NOP)
	AM_RANGE(0x3c00, 0x3cff) AM_MIRROR(0x03ff) AM_READWRITE(input_port_1_r, MWA8_NOP)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( safarir )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x04, "Acceleration Rate" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x0c, "Fastest" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x40, "7000" )
	PORT_DIPSETTING(    0x60, "9000" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( safarir )

 	/* basic machine hardware */
	MDRV_CPU_ADD(8080, 3072000)	/* 3 MHz ? */
	MDRV_CPU_PROGRAM_MAP(main_map, 0)

	MDRV_MACHINE_START(safarir)

	/* video hardware */
	MDRV_VIDEO_START(safarir)
	MDRV_VIDEO_UPDATE(safarir)
	MDRV_PALETTE_INIT(safarir)
	MDRV_PALETTE_LENGTH(2*8)
	MDRV_GFXDECODE(safarir)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 26*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)

	/* audio hardware */

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( safarir )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rl01", 0x0000, 0x0400, CRC(cf7703c9) SHA1(b4182df9332b355edaa518462217a6e31e1c07b2) )
	ROM_LOAD( "rl02", 0x0400, 0x0400, CRC(1013ecd3) SHA1(2fe367db8ca367b36c5378cb7d5ff918db243c78) )
	ROM_LOAD( "rl03", 0x0800, 0x0400, CRC(84545894) SHA1(377494ceeac5ad58b70f77b2b27b609491cb7ffd) )
	ROM_LOAD( "rl04", 0x0c00, 0x0400, CRC(5dd12f96) SHA1(a80ac0705648f0807ea33e444fdbea450bf23f85) )
	ROM_LOAD( "rl05", 0x1000, 0x0400, CRC(935ed469) SHA1(052a59df831dcc2c618e9e5e5fdfa47548550596) )
	ROM_LOAD( "rl06", 0x1400, 0x0400, CRC(24c1cd42) SHA1(fe32ecea77a3777f8137ca248b8f371db37b8b85) )

	ROM_REGION( 0x0400, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rl08", 0x0000, 0x0400, CRC(d6a50aac) SHA1(0a0c2cefc556e4e15085318fcac485b82bac2416) )

	ROM_REGION( 0x0400, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "rl07", 0x0000, 0x0400, CRC(ba525203) SHA1(1c261cc1259787a7a248766264fefe140226e465) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1979, safarir, 0, safarir, safarir, 0, ROT90, "SNK", "Safari Rally (Japan)", GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
