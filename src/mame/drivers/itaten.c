/*

Itazura Tenshi (Japan Ver.)
(c)1984 Nichibutsu / Alice



--- Team Japump!!! ---
Dumped by Chack'n
01/Aug/2011
Written by Hau
05/Aug/2011

based on driver from drivers/dacholer.c by Pierpaolo Prazzoli
note:
Sound test will not work.
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

class itaten_state : public driver_device
{
public:
	itaten_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this,"audiocpu")
		{ }

	/* memory pointers */
	UINT8 *  m_bgvideoram;
	UINT8 *  m_fgvideoram;
	UINT8 *  m_spriteram;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8    m_scroll_x;
	UINT8    m_scroll_y;
	int      m_bg_bank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
};



/*************************************
 *
 *  Video system
 *
 *************************************/

/* guess: use the same resistor values as Crazy Climber (needs checking on the real HW) */
static PALETTE_INIT( itaten )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, 0, 0, 0, 0);

	for (i = 0;i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


static TILE_GET_INFO( get_bg_tile_info )
{
	itaten_state *state = machine.driver_data<itaten_state>();
	SET_TILE_INFO(1, state->m_bgvideoram[tile_index] + state->m_bg_bank * 0x100, 0, 0);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	itaten_state *state = machine.driver_data<itaten_state>();
	SET_TILE_INFO(0, state->m_fgvideoram[tile_index], 0, 0);
}


static VIDEO_START( itaten )
{
	itaten_state *state = machine.driver_data<itaten_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);
}


static WRITE8_HANDLER( background_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

static WRITE8_HANDLER( foreground_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	state->m_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

static WRITE8_HANDLER( bg_bank_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();

	if ((data & 3) != state->m_bg_bank)
	{
		state->m_bg_bank = data & 3;
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
	}

	flip_screen_set(space->machine(), data & 0xc); // probably one bit for flipx and one for flipy
}


static WRITE8_HANDLER( bg_scroll_x_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	state->m_scroll_x = data;
}

static WRITE8_HANDLER( bg_scroll_y_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	state->m_scroll_y = data;
}


static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	itaten_state *state = machine.driver_data<itaten_state>();
	int offs, code, attr, sx, sy, flipx, flipy;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		code = state->m_spriteram[offs + 1];
		attr = state->m_spriteram[offs + 2];

		flipx = attr & 0x10;
		flipy = attr & 0x20;

		sx = (state->m_spriteram[offs + 3] - 128) + 256 * (attr & 0x01);
		sy = 255 - state->m_spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[2],
				code,
				0,
				flipx,flipy,
				sx,sy,0);
	}
}


static SCREEN_UPDATE(itaten)
{
	itaten_state *state = screen->machine().driver_data<itaten_state>();

	if (flip_screen_get(screen->machine()))
	{
		tilemap_set_scrollx(state->m_bg_tilemap, 0, 256 - state->m_scroll_x);
		tilemap_set_scrolly(state->m_bg_tilemap, 0, 256 - state->m_scroll_y);
	}
	else
	{
		tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_scroll_x);
		tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_scroll_y);
	}

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	return 0;
}


/*************************************/

static WRITE8_HANDLER( coins_w )
{
	coin_counter_w(space->machine(), 0, data & 1);
	coin_counter_w(space->machine(), 1, data & 2);

	set_led_status(space->machine(), 0, data & 4);
	set_led_status(space->machine(), 1, data & 8);
}


static WRITE8_HANDLER(snd_w)
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	soundlatch_w(space, offset, data);
	device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( main_irq_ack_w )
{
	itaten_state *state = space->machine().driver_data<itaten_state>();
	device_set_input_line(state->m_maincpu, 0, CLEAR_LINE);
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xb7ff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x0400) AM_RAM_WRITE(background_w) AM_BASE_MEMBER(itaten_state, m_bgvideoram)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(foreground_w) AM_BASE_MEMBER(itaten_state, m_fgvideoram)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_BASE_SIZE_MEMBER(itaten_state, m_spriteram, m_spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSWA")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWB")
	AM_RANGE(0x05, 0x05) AM_READNOP			/* watchdog */
	AM_RANGE(0x20, 0x20) AM_WRITE(coins_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(bg_bank_w)
	AM_RANGE(0x22, 0x22) AM_WRITE(bg_scroll_x_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(bg_scroll_y_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(main_irq_ack_w)
	AM_RANGE(0x27, 0x27) AM_WRITE(snd_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snd_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( snd_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_r, soundlatch_clear_w )
	AM_RANGE(0x86, 0x87) AM_DEVWRITE("ay1", ay8910_data_address_w)
	AM_RANGE(0x8a, 0x8b) AM_DEVWRITE("ay2", ay8910_data_address_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVWRITE("ay3", ay8910_data_address_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( itaten )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "30k then every 50k" )
	PORT_DIPSETTING(    0x20, "60k then every 50k" )
	PORT_DIPSETTING(    0x10, "30k then every 90k" )
	PORT_DIPSETTING(    0x00, "60k then every 90k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

static GFXDECODE_START( itaten )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x10, 1 )
GFXDECODE_END



static MACHINE_START( itaten )
{
	itaten_state *state = machine.driver_data<itaten_state>();

	state->save_item(NAME(state->m_scroll_x));
	state->save_item(NAME(state->m_scroll_y));
	state->save_item(NAME(state->m_bg_bank));
}


static MACHINE_CONFIG_START( itaten, itaten_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_19_968MHz/8)		/* Matches reference recording */
	MCFG_CPU_PROGRAM_MAP(snd_map)
	MCFG_CPU_IO_MAP(snd_io_map)

	MCFG_MACHINE_START(itaten)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1-16)
	MCFG_SCREEN_UPDATE(itaten)

	MCFG_PALETTE_LENGTH(32)
	MCFG_PALETTE_INIT(itaten)
	MCFG_GFXDECODE(itaten)

	MCFG_VIDEO_START(itaten)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_19_968MHz/16)	/* Matches reference recording */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_19_968MHz/16)	/* Matches reference recording */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay3", AY8910, XTAL_19_968MHz/16)	/* Matches reference recording */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition
 *
 *************************************/

/*
--------------------------------
IT A-1
CPU  :LH0080 Z80,LH0080A Z80A
Sound:AY-3-8910 x3
OSC  :16.000MHz
--------------------------------
1.5K         [84c8a010] 2764
2.5L         [19946038]  |
3.5M         [4f9e26fd] /

6.6G         [dfcb1a3e] 2764
7.6H         [844e78d6] 2732

AF-1.3N      [5638e485] 82S123


--------------------------------
ITA-EXP
--------------------------------
4.1F         [35f85aeb] 2764
5.1E         [6cf30924] /


--------------------------------
IT A-2
OSC  :19.968 ?
--------------------------------
8.10A        [c32b0859] 2764
9.11A        [919cac5e]  |
10.12A       [d2b60e5d]  |
11.13A       [ed3279d5] /

12.1D        [f0f64636] 2764
13.2D        [d32559f5]  |
14.3D        [8c532c74]  |
15.4D        [d119b483] /

16.12J       [8af2bfb8] 2764

AF-2.1H      [e1cac297] 82S123
AF-3.13D     [875429ba] /
--------------------------------
*/

ROM_START( itaten )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5k",  0x0000, 0x2000, CRC(84c8a010) SHA1(52d78ac70b3d5e905a11efd76acd99810c56e467) )
	ROM_LOAD( "2.5l",  0x2000, 0x2000, CRC(19946038) SHA1(74f76096e676535ead4386755fce853caac7673b) )
	ROM_LOAD( "3.5m",  0x4000, 0x2000, CRC(4f9e26fd) SHA1(33062724c46108611c9db16fdbf2cb9feed7e213) )
	ROM_LOAD( "4.1f",  0x6000, 0x2000, CRC(35f85aeb) SHA1(ecd8f62e304d1277332a5a2b7ec6aace9f77d8ad) )
	ROM_LOAD( "5.1e",  0x8000, 0x2000, CRC(6cf30924) SHA1(5e82e9aa0811ec1b853d300368c5ceec44938363) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.6g",  0x0000, 0x2000, CRC(dfcb1a3e) SHA1(cee0906cfbddd0254a947737da1cfbe47c445c32) )
	ROM_LOAD( "7.6h",  0x2000, 0x1000, CRC(844e78d6) SHA1(11b48af650809f8504b56e9a8e53c9f043782c5f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "16.12j", 0x0000, 0x2000, CRC(8af2bfb8) SHA1(6744db0deb4fda7920fcfddf7f9c1ed6681d3622) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "11.13a", 0x0000, 0x2000, CRC(ed3279d5) SHA1(e4bcae8038739c588f896ff35fa95288979fa683) )
	ROM_LOAD( "10.12a", 0x2000, 0x2000, CRC(d2b60e5d) SHA1(c833ac6e5d4d0a244ace600f5f02c6f43d3bd34b) )
	ROM_LOAD( "9.11a",  0x4000, 0x2000, CRC(919cac5e) SHA1(9602ad7618b5e4d93fa22676aa855256d4690dd1) )
	ROM_LOAD( "8.10a",  0x6000, 0x2000, CRC(c32b0859) SHA1(1bb00de55742a1f2cbbf3970b43b122114b0911b) )

	ROM_REGION( 0x8000, "gfx3", 0 )
	ROM_LOAD( "13.2d",  0x0000, 0x2000, CRC(d32559f5) SHA1(a4f05b1c8c48aad367ff675c29a9a1828c71b693) )
	ROM_LOAD( "12.1d",  0x2000, 0x2000, CRC(f0f64636) SHA1(a3354be74460e45453fea62c8dd910f98b5d2fb5) )
	ROM_LOAD( "14.3d",  0x4000, 0x2000, CRC(8c532c74) SHA1(c95786c81f82f7211f4411a9f39fd4ba4def9073) )
	ROM_LOAD( "15.4d",  0x6000, 0x2000, CRC(d119b483) SHA1(c1e403369bfbda0233ec5764fc703522e9f312a7) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "af-3.13d", 0x0000, 0x0020, CRC(875429ba) SHA1(7186e1fb15806d60fd3e704be8db94c7b6c8c058) )
	ROM_LOAD( "af-2.1h",  0x0020, 0x0020, CRC(e1cac297) SHA1(f15326d04d006d9d029a6565aebf9daf3657bc2a) )
	ROM_LOAD( "af-1.3n",  0x0040, 0x0020, CRC(5638e485) SHA1(5d892111936a8eb7646c03a17300069be9a2b442) )
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1984, itaten, 0, itaten, itaten, 0, ROT0, "Nichibutsu / Alice", "Itazura Tenshi (Japan)", GAME_SUPPORTS_SAVE )


