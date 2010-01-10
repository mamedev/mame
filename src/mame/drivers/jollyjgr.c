/*

Jolly Jogger
Taito, 1982

PCB Layout
----------

FGO70007
KDN00004
KDK00502
|----------------------------------------------------------|
|             TMM416  TMM416  TMM416  TMM416               |
|             TMM416  TMM416  TMM416  TMM416              |-|
|             TMM416  TMM416  TMM416  TMM416              | |
|             TMM416  TMM416  TMM416  TMM416              | |
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|2                                                        |-|
|2                                                         |
|W   DSW1(8)                                     KD13.1F   |
|A                                                         |
|Y                                                         |
|                                                         |-|
|                                                         | |
|                                                         | |
|                                                         | |
|                MB14241                                  | |
|    DSW2(8)                                              | |
|                                                         | |
|                                                         |-|
|    VOL         AY3-8910     3.579545MHz                  |
|----------------------------------------------------------|

FGO70008
KDN00007
|----------------------------------------------------------|
|                                                          |
|                                            6116         |-|
|                                                         | |
|                                            KD21.8J      | |
|   D2125          D2125                                  | |
|                                            KD20.8H      | |
|   D2125          D2125                                  | |
|                                            KD19.8F      | |
|   D2125          D2125                                  |-|
|                                            KD18.8E       |
|   D2125          D2125                                   |
|                                            KD17.8D       |
|   D2125          D2125        Z80                        |
|                                            KD16.8C      |-|
|                                                         | |
|                                            KD15.8B      | |
|                                                         | |
|                                            KD14.8A      | |
|                                                         | |
|                                                         | |
|                                                         |-|
|                                                          |
|----------------------------------------------------------|

FGO70009
KDN00006
|----------------------------------------------------------|
|                                                18MHz     |
|                                                         |-|
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|                         KD11.5H   KD12.7H               | |
|                                                         |-|
|1                                                         |
|8                                                         |
|W                                                         |
|A                                                         |
|Y                                                        |-|
|                                                         | |
|                                                         | |
|                                                         | |
|   KD09.1C  KD10.2C                                      | |
|                                                         | |
|                                                         | |
|               2114  2114     2114  2114  2114  2114     |-|
|                                                          |
|----------------------------------------------------------|


  driver by Pierpaolo Prazzoli

Notes:
  The hardware is very similar to Galaxian but has some differencies, like the 3bpp bitmap addition
  To advance input tests, press Tilt button

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


typedef struct _jollyjgr_state jollyjgr_state;
struct _jollyjgr_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  bitmap;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	int      nmi_enable, flip_x, flip_y;
};


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( jollyjgr_videoram_w )
{
	jollyjgr_state *state = (jollyjgr_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static WRITE8_HANDLER( jollyjgr_attrram_w )
{
	jollyjgr_state *state = (jollyjgr_state *)space->machine->driver_data;

	if (offset & 1)
	{
		/* color change */
		int i;

		for (i = offset >> 1; i < 0x0400; i += 32)
			tilemap_mark_tile_dirty(state->bg_tilemap, i);
	}
	else
	{
		tilemap_set_scrolly(state->bg_tilemap, offset >> 1, data);
	}

	state->colorram[offset] = data;
}

static WRITE8_HANDLER( jollyjgr_misc_w )
{
	jollyjgr_state *state = (jollyjgr_state *)space->machine->driver_data;

	// they could be swapped, because it always set "data & 3"
	state->flip_x = data & 1;
	state->flip_y = data & 2;

	tilemap_set_flip(state->bg_tilemap, (state->flip_x ? TILEMAP_FLIPX : 0) | (state->flip_y ? TILEMAP_FLIPY : 0));

	state->nmi_enable = data & 0x80;
}

static WRITE8_HANDLER( jollyjgr_coin_lookout_w )
{
	coin_lockout_global_w(space->machine, data & 1);

	/* bits 4, 5, 6 and 7 are used too */
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( jollyjgr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8ff8, 0x8ff8) AM_READ_PORT("DSW1")
	AM_RANGE(0x8ff9, 0x8ff9) AM_READ_PORT("INPUTS")
	AM_RANGE(0x8ff8, 0x8ff8) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0x8ffa, 0x8ffa) AM_READ_PORT("SYSTEM") AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x8fff, 0x8fff) AM_READ_PORT("DSW2")
	AM_RANGE(0x8ffc, 0x8ffc) AM_WRITE(jollyjgr_misc_w)
	AM_RANGE(0x8ffd, 0x8ffd) AM_WRITE(jollyjgr_coin_lookout_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(jollyjgr_videoram_w) AM_BASE_MEMBER(jollyjgr_state, videoram)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(jollyjgr_attrram_w) AM_BASE_MEMBER(jollyjgr_state, colorram)
	AM_RANGE(0x9840, 0x987f) AM_RAM AM_BASE_MEMBER(jollyjgr_state, spriteram)
	AM_RANGE(0x9880, 0x9bff) AM_RAM
	AM_RANGE(0xa000, 0xffff) AM_RAM AM_BASE_MEMBER(jollyjgr_state, bitmap)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jollyjgr )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Timer" )
	PORT_DIPSETTING(    0x18, "2 min 20 sec" )
	PORT_DIPSETTING(    0x10, "2 min 40 sec" )
	PORT_DIPSETTING(    0x08, "3 min" )
	PORT_DIPSETTING(    0x00, "3 min 20 sec" )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) ) // it works only when Cabinet is set to Upright
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x00, "Display Coinage" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Display Year" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "No Hit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Number of Coin Switches" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )
INPUT_PORTS_END

/*************************************
 *
 *  Video emulation
 *
 *************************************/

static PALETTE_INIT( jollyjgr )
{
	int i;

	/* tilemap / sprites palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* bitmap palette */
	for (i = 0;i < 8;i++)
		palette_set_color_rgb(machine, 32 + i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}

/* Tilemap is the same as in Galaxian */
static TILE_GET_INFO( get_bg_tile_info )
{
	jollyjgr_state *state = (jollyjgr_state *)machine->driver_data;
	int color = state->colorram[((tile_index & 0x1f) << 1) | 1] & 7;
	SET_TILE_INFO(0, state->videoram[tile_index], color, 0);
}

static VIDEO_START( jollyjgr )
{
	jollyjgr_state *state = (jollyjgr_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->bg_tilemap, 0);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);
}

static void draw_bitmap( running_machine *machine, bitmap_t *bitmap )
{
	jollyjgr_state *state = (jollyjgr_state *)machine->driver_data;
	int x, y, count;
	int i, bit0, bit1, bit2;
	int color;

	count = 0;
	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256 / 8; x++)
		{
			for(i = 0; i < 8; i++)
			{
				bit0 = (state->bitmap[count] >> i) & 1;
				bit1 = (state->bitmap[count + 0x2000] >> i) & 1;
				bit2 = (state->bitmap[count + 0x4000] >> i) & 1;
				color = bit0 | (bit1 << 1) | (bit2 << 2);

				if(color)
				{
					if(state->flip_x && state->flip_y)
						*BITMAP_ADDR16(bitmap, y, x * 8 + i) = color + 32;
					else if(state->flip_x && !state->flip_y)
						*BITMAP_ADDR16(bitmap, 255 - y, x * 8 + i) = color + 32;
					else if(!state->flip_x && state->flip_y)
						*BITMAP_ADDR16(bitmap, y, 255 - x * 8 - i) = color + 32;
					else
						*BITMAP_ADDR16(bitmap, 255 - y, 255 - x * 8 - i) = color + 32;
				}
			}

			count++;
		}
	}
}

static VIDEO_UPDATE( jollyjgr )
{
	jollyjgr_state *state = (jollyjgr_state *)screen->machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	bitmap_fill(bitmap, cliprect, 32);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	draw_bitmap(screen->machine, bitmap);

	/* Sprites are the same as in Galaxian */
	for (offs = 0; offs < 0x40; offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = spriteram[offs];
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy = spriteram[offs + 1] & 0x80;
		int code = spriteram[offs + 1] & 0x3f;
		int color = spriteram[offs + 2] & 7;

		if (state->flip_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (state->flip_y)
			flipy = !flipy;
		else
			sy = 240 - sy;

		if (offs < 3 * 4)
			sy++;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
				code,color,
				flipx,flipy,
				sx,sy,0);

	}
	return 0;
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout jollyjgr_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout jollyjgr_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( jollyjgr )
	GFXDECODE_ENTRY( "gfx1", 0, jollyjgr_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, jollyjgr_spritelayout, 0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static INTERRUPT_GEN( jollyjgr_interrupt )
{
	jollyjgr_state *state = (jollyjgr_state *)device->machine->driver_data;
	if(state->nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_START( jollyjgr )
{
	jollyjgr_state *state = (jollyjgr_state *)machine->driver_data;

	state_save_register_global(machine, state->nmi_enable);
	state_save_register_global(machine, state->flip_x);
	state_save_register_global(machine, state->flip_y);
}

static MACHINE_RESET( jollyjgr )
{
	jollyjgr_state *state = (jollyjgr_state *)machine->driver_data;

	state->nmi_enable = 0;
	state->flip_x = 0;
	state->flip_y = 0;
}

static MACHINE_DRIVER_START( jollyjgr )

	/* driver data */
	MDRV_DRIVER_DATA(jollyjgr_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 3579545)		 /* 3,579545 MHz */
	MDRV_CPU_PROGRAM_MAP(jollyjgr_map)
	MDRV_CPU_VBLANK_INT("screen", jollyjgr_interrupt)

	MDRV_MACHINE_START(jollyjgr)
	MDRV_MACHINE_RESET(jollyjgr)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(jollyjgr)
	MDRV_PALETTE_LENGTH(32+8) /* 32 for tilemap and sprites + 8 for the bitmap */

	MDRV_PALETTE_INIT(jollyjgr)
	MDRV_VIDEO_START(jollyjgr)
	MDRV_VIDEO_UPDATE(jollyjgr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jollyjgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kd14.8a",      0x0000, 0x1000, CRC(404cfa2b) SHA1(023abecbc614d1deb6a239906f62e25bb688ac14) )
	ROM_LOAD( "kd15.8b",      0x1000, 0x1000, CRC(4cdc4c8b) SHA1(07257863a2de3a0e6bc1b41b8dcaae8c89bc4720) )
	ROM_LOAD( "kd16.8c",      0x2000, 0x1000, CRC(a2fa3500) SHA1(b85439e43a31c3445420896c231ac59f95331226) )
	ROM_LOAD( "kd17.8d",      0x3000, 0x1000, CRC(a5ab8c89) SHA1(b5a41581bba1c26ac3819aded29aeb48a5de64f8) )
	ROM_LOAD( "kd18.8e",      0x4000, 0x1000, CRC(8547c9a7) SHA1(43319a2271a294c2876e87a30bb7667b67582b76) )
	ROM_LOAD( "kd19.8f",      0x5000, 0x1000, CRC(2d0ed544) SHA1(547bcecd7d97f343de721d6af5f13ae6f787d838) )
	ROM_LOAD( "kd20.8h",      0x6000, 0x1000, CRC(017a0e5a) SHA1(23e066fea44690279ff7b3b65b03e4096e4d2984) )
	ROM_LOAD( "kd21.8j",      0x7000, 0x1000, CRC(e4faed01) SHA1(9b9afaff6cc4addfed7c1929a0d845bfbe9d18cc) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "kd09.1c",      0x0000, 0x0800, CRC(ecafd535) SHA1(a1f0bee247e6ab4f9fc3578560b62f5913b4ece2) )
	ROM_LOAD( "kd10.2c",      0x0800, 0x0800, CRC(e40fc594) SHA1(1a9bd670dda0405600e4c4d0107b881906969991) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "kd11.5h",      0x0000, 0x0800, CRC(d686245c) SHA1(73567b15d9399e450121ad01ad2dcb91bedc1099) )
	ROM_LOAD( "kd12.7h",      0x0800, 0x0800, CRC(d69cbb4e) SHA1(f33cc161f93cae9cc314067fa2453838fa8ac3ba) )

	/* it's identical to kd14.8a, except for the first 32 bytes which are palette bytes */
	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "kd13.1f",      0x0000, 0x1000, CRC(4f4e4e13) SHA1(a8fe0e1fd354e6cc2cf65eab66882c3b98c82100) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, jollyjgr, 0, jollyjgr, jollyjgr, 0, ROT90, "Taito Corporation", "Jolly Jogger", GAME_SUPPORTS_SAVE )
