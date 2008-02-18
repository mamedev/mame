/***************************************************************************

Oli-Boo-Chu

driver by Nicola Salmoria

TODO:
- main->sound cpu communication is completely wrong, commands don't play the
  intended sound.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"

static tilemap *bg_tilemap;

static PALETTE_INIT( olibochu )
{
	int i;

	for (i = 0; i < machine->config->total_colors; i++)
	{
		UINT8 pen;
		int bit0, bit1, bit2, r, g, b;

		if (i < 0x100)
			/* characters */
			pen = (color_prom[0x020 + (i - 0x000)] & 0x0f) | 0x10;
		else
			/* sprites */
			pen = (color_prom[0x120 + (i - 0x100)] & 0x0f) | 0x00;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[pen] >> 6) & 0x01;
		bit1 = (color_prom[pen] >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static WRITE8_HANDLER( olibochu_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( olibochu_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( olibochu_flipscreen_w )
{
	if (flip_screen != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* other bits are used, but unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x20) << 3);
	int color = (attr & 0x1f) + 0x20;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

static VIDEO_START( olibochu )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* 16x16 sprites */

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int attr = spriteram[offs+1];
		int code = spriteram[offs];
		int color = attr & 0x3f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs+3];
		int sy = ((spriteram[offs+2] + 8) & 0xff) - 8;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}

	/* 8x8 sprites */

	for (offs = 0;offs < spriteram_2_size;offs += 4)
	{
		int attr = spriteram_2[offs+1];
		int code = spriteram_2[offs];
		int color = attr & 0x3f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram_2[offs+3];
		int sy = spriteram_2[offs+2];

		if (flip_screen)
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[0],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

static VIDEO_UPDATE( olibochu )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}


static WRITE8_HANDLER( sound_command_w )
{
	static int cmd;
	int c;


	if (offset == 0) cmd = (cmd & 0x00ff) | (data << 8);
	else cmd = (cmd & 0xff00) | data;

	for (c = 15;c >= 0;c--)
		if (cmd & (1 << c)) break;

	if (c >= 0) soundlatch_w(0,15-c);
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa001, 0xa001) AM_READ(input_port_1_r)
	AM_RANGE(0xa002, 0xa002) AM_READ(input_port_2_r)
	AM_RANGE(0xa003, 0xa003) AM_READ(input_port_3_r)
	AM_RANGE(0xa004, 0xa004) AM_READ(input_port_4_r)
	AM_RANGE(0xa005, 0xa005) AM_READ(input_port_5_r)
	AM_RANGE(0xf000, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(olibochu_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(olibochu_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xa800, 0xa801) AM_WRITE(sound_command_w)
	AM_RANGE(0xa802, 0xa802) AM_WRITE(olibochu_flipscreen_w)	/* bit 6 = enable sound? */
	AM_RANGE(0xf000, 0xffff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xf400, 0xf41f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xf440, 0xf47f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2) AM_SIZE(&spriteram_2_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x63ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x7000, 0x7000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x6000, 0x63ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(AY8910_write_port_0_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( olibochu )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )	/* works in service mode but not in game */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW0") /* Listed as sw1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) /* Nothing listed for this DIP */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) /* Nothing listed for this DIP */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Cross Hatch Pattern" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW1") /* Most likely not a bank of Dip Switches */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2") /* Listed as sw2 */
	PORT_DIPNAME( 0x01, 0x01, "Stop Mode (Cheat)") /* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, "Invulnerability (Cheat)" ) /* Listed as "No Hit" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) /* Listed as "Start Pattern"... Level Select or Preview?? */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) /* Listed as "Screen 180" currently has no effect */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( olibochu )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 256, 64 )
GFXDECODE_END




static INTERRUPT_GEN( olibochu_interrupt )
{
	if (cpu_getiloops() == 0)
		cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0xcf);	/* RST 08h */
	else
		cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0xd7);	/* RST 10h */
}

static MACHINE_DRIVER_START( olibochu )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)	/* 4 MHz ?? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(olibochu_interrupt,2)

	MDRV_CPU_ADD(Z80, 4000000)
	/* audio CPU */	/* 4 MHz ?? */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(olibochu)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(olibochu)
	MDRV_VIDEO_START(olibochu)
	MDRV_VIDEO_UPDATE(olibochu)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( olibochu )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* main CPU */
	ROM_LOAD( "1b.3n",        0x0000, 0x1000, CRC(bf17f4f4) SHA1(1075456f4b70a68548e0e1b6271fd4b845a77ce4) )
	ROM_LOAD( "2b.3lm",       0x1000, 0x1000, CRC(63833b0d) SHA1(0135c449c92470241d03a87709c739209139d660) )
	ROM_LOAD( "3b.3k",        0x2000, 0x1000, CRC(a4038e8b) SHA1(d7dce830239c8975ac135b213a99eec0c20ec3e2) )
	ROM_LOAD( "4b.3j",        0x3000, 0x1000, CRC(aad4bec4) SHA1(9203564ac841a8de2f9b8183d4086acce95e3d47) )
	ROM_LOAD( "5b.3h",        0x4000, 0x1000, CRC(66efa79f) SHA1(535369d958461834435d3202cd7310ecd0aa528c) )
	ROM_LOAD( "6b.3f",        0x5000, 0x1000, CRC(1123d1ef) SHA1(6094e732e61915c45b14acd90c1343f05385daf4) )
	ROM_LOAD( "7c.3e",        0x6000, 0x1000, CRC(89c26fb4) SHA1(ebc51e40612af894b20bd7fc3a5179cd35aaac9b) )
	ROM_LOAD( "8b.3d",        0x7000, 0x1000, CRC(af19e5a5) SHA1(5a55bbee5b2f20e2988171a310c8293dabbd9a72) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound CPU */
	ROM_LOAD( "17.4j",        0x0000, 0x1000, CRC(57f07402) SHA1(a763a835ac512c69b4351c1ec72b0a64e46203aa) )
	ROM_LOAD( "18.4l",        0x1000, 0x1000, CRC(0a903e9c) SHA1(d893c2f5373f748d8bebf3673b15014f4a8d4b5c) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )	/* samples? */
	ROM_LOAD( "15.1k",        0x0000, 0x1000, CRC(fb5dd281) SHA1(fba947ae7b619c2559b5af69ef02acfb15733f0d) )
	ROM_LOAD( "16.1m",        0x1000, 0x1000, CRC(c07614a5) SHA1(d13d271a324f99d008429c16193c4504e5894493) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "13.6n",        0x0000, 0x1000, CRC(b4fcf9af) SHA1(b360daa0670160dca61512823c98bc37ad99b9cf) )
	ROM_LOAD( "14.4n",        0x1000, 0x1000, CRC(af54407e) SHA1(1883928b721e03e452fd0c626c403dc374b02ed7) )

	ROM_REGION( 0x4000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "9.6a",         0x0000, 0x1000, CRC(fa69e16e) SHA1(5a493a0a108b3e496884d1f499f3445d4e241ecd) )
	ROM_LOAD( "10.2a",        0x1000, 0x1000, CRC(10359f84) SHA1(df55f06fd98233d0efbc30e3e24bf9b8cab1a5cc) )
	ROM_LOAD( "11.4a",        0x2000, 0x1000, CRC(1d968f5f) SHA1(4acf78d865ca36355bb15dc1d476f5e97a5d91b7) )
	ROM_LOAD( "12.2a",        0x3000, 0x1000, CRC(d8f0c157) SHA1(a7b0c873e016c3b3252c2c9b6400b0fd3d650b2f) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "c-1",          0x0000, 0x0020, CRC(e488e831) SHA1(6264741f7091c614093ae1ea4f6ead3d0cef83d3) )	/* palette */
	ROM_LOAD( "c-2",          0x0020, 0x0100, CRC(698a3ba0) SHA1(3c1a6cb881ef74647c651462a27d812234408e45) )	/* sprite lookup table */
	ROM_LOAD( "c-3",          0x0120, 0x0100, CRC(efc4e408) SHA1(f0796426cf324791853aa2ae6d0c3d1f8108d5c2) )	/* char lookup table */
ROM_END



GAME( 1981, olibochu, 0, olibochu, olibochu, 0, ROT270, "Irem + GDI", "Oli-Boo-Chu", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
