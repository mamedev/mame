/*
    Talbot (c) 1982 Volt Electronics

ALPHA 8201 MCU handling by Tatsuyuki satoh

    TODO:
        verify CPU and MCU clock frequency.
*/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"

static tilemap *bg_tilemap;

static WRITE8_HANDLER( talbot_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( talbot_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( talbot_mcu_halt_w )
{
	data &= 1;
	cpunum_set_input_line(Machine, 1, INPUT_LINE_HALT, data ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE8_HANDLER( talbot_mcu_switch_w )
{
}

static ADDRESS_MAP_START( cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE(1) /* MCU shared RAM */

	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8000, 0x83ff) AM_READWRITE(MRA8_RAM, talbot_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x8400, 0x87ff) AM_READWRITE(MRA8_RAM, talbot_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8800, 0x8fef) AM_RAM
	AM_RANGE(0x8ff0, 0x8fff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa006, 0xa006) AM_WRITE(talbot_mcu_halt_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITE(talbot_mcu_switch_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa040, 0xa040) AM_READ(input_port_1_r)
	AM_RANGE(0xa060, 0xa06f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0xa080, 0xa080) AM_READ(input_port_2_r)
	AM_RANGE(0xa0c0, 0xa0c0) AM_READWRITE(input_port_3_r,MWA8_NOP /*watchdog_reset_w*/)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE(1) /* main CPU shared RAM */
ADDRESS_MAP_END

static INPUT_PORTS_START( talbot )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN	              )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN                )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Has to be 0 */

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },	/* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 bytes */
};

static GFXDECODE_START( talbot )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 0, 64 )
GFXDECODE_END

static TILE_GET_INFO( get_tile_info_bg )
{
	int code = videoram[tile_index];
	int color = colorram[tile_index] & 0x1f;

	SET_TILE_INFO(0, code, color + 0x20, 0);
}

static VIDEO_START( talbot )
{
	bg_tilemap = tilemap_create(get_tile_info_bg, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int code = spriteram[offs] >> 2;
		int color = spriteram[offs + 1];
		int flipx = spriteram[offs] & 0x01;
		int flipy = spriteram[offs] & 0x02;
		int sx = ((256 + 16 - spriteram_2[offs + 1]) & 0xff) - 16;
		int sy = spriteram_2[offs] - 16;

		drawgfx(bitmap,
			machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

static VIDEO_UPDATE( talbot )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}

static PALETTE_INIT( talbot )
{
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */

	/* Colortable entry */
	for(i = 0; i < 0x100; i++)
		colortable[i] = color_prom[i];
}

static MACHINE_DRIVER_START( talbot )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 18432000/6)	/* (?) */
	MDRV_CPU_PROGRAM_MAP(cpu_map, 0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold, 1)

	/* MCU */
//  MDRV_CPU_ADD(ALPHA8201, 18432000/6/8/2) /* (?) */
	MDRV_CPU_ADD(ALPHA8201, 18432000/6/8)
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(talbot)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(0x100)

	MDRV_PALETTE_INIT(talbot)
	MDRV_VIDEO_START(talbot)
	MDRV_VIDEO_UPDATE(talbot)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8910, 18432000/12)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( talbot )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "11.10g", 0x0000, 0x1000, CRC(0368607d) SHA1(275a29fb018bd327e64cf4fcc04590099c90290a) )
	ROM_LOAD( "12.11g", 0x1000, 0x1000, CRC(400e633b) SHA1(8d76df34174286e2b0c9341bbc141c9e77533f06) )
	ROM_LOAD( "13.10h", 0x2000, 0x1000, CRC(be575d9e) SHA1(17d3bbdc755920b5a6e1e81cbb7d51be20257ff1) )
	ROM_LOAD( "14.11h", 0x3000, 0x1000, CRC(56464614) SHA1(21cfcf3212e0a74c695ce1d6412d630a7141b2c9) )
	ROM_LOAD( "15.10i", 0x4000, 0x1000, CRC(0225b7ef) SHA1(9adee4831eb633b0a31580596205a655df94c2b2) )
	ROM_LOAD( "16.11i", 0x5000, 0x1000, CRC(1612adf5) SHA1(9adeb21d5d1692f6e31460062f03f2008076b307) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "7.6a", 0x0000, 0x1000, CRC(bde14194) SHA1(f8f569342a3094eb5450a30b8ab87901b98e6061) )

	ROM_REGION( 0x1000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "8.6b", 0x0000, 0x1000, CRC(ddcd227a) SHA1(c44de36311cd173afb3eebf8487305b06e069c0f) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "mb7051.7h", 0x0000, 0x0020, CRC(7a153c60) SHA1(4b147c63e467cca7359acb5f3652ed9db9a36cc8) )
	ROM_LOAD( "mb7052.5e", 0x0020, 0x0100, CRC(a3189986) SHA1(f113c1253ba2f8f213c600e93a39c0957a933306) )
ROM_END

GAME( 1982, talbot, 0, talbot, talbot, 0, ROT90, "Volt Electronics (Alpha license)", "Talbot", 0 )
