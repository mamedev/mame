/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/crtc6845.h"
#include "sound/ay8910.h"

#define MAIN_CLOCK 10595000

#define SOUND_CLOCK 4000000

#define PIXEL_CLOCK (MAIN_CLOCK / 2)


static UINT8* madalien_videoram;
static UINT8* madalien_charram;

static UINT8 madalien_video_flags;
static UINT8 madalien_screen_control;
static UINT8 madalien_scroll;
static UINT8 madalien_edge1_pos;
static UINT8 madalien_edge2_pos;
static UINT8 madalien_headlight_pos;
static UINT8 madalien_shift_count;
static UINT8 madalien_shift_data;

static tilemap* tilemap_fg;

static tilemap* tilemap_edge1[4];
static tilemap* tilemap_edge2[4];

static mame_bitmap* tmp_bitmap;
static mame_bitmap* headlight_bitmap;


static PALETTE_INIT( madalien )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		int r = 0;
		int g = 0;
		int b = 0;

		if (BIT(color_prom[i], 0))
			r += 0x3f;
		if (BIT(color_prom[i], 1))
			r += 0xc0;
		if (BIT(color_prom[i], 2))
			g += 0x3f;
		if (BIT(color_prom[i], 3))
			g += 0xc0;
		if (BIT(color_prom[i], 4))
			b += 0x3f;
		if (BIT(color_prom[i], 5))
			b += 0xc0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 16; i++)
	{
		colortable[0x00 + i] = i;
		colortable[0x10 + i] = i;
		colortable[0x20 + i] = i + 0x10;

		/* PCB can swap address lines 1 and 2 when accessing color PROM */

		if (BIT(i, 1))
		{
			colortable[0x10 + i] ^= 6;
		}
		if (BIT(i, 2))
		{
			colortable[0x10 + i] ^= 6;
		}
	}
}


static void update_edges(int area)
{
	UINT8* map = memory_region(REGION_GFX2) + 0x200 * area;

	// MADALIEN:
	//
	//   area 0 section A - casino
	//          section B - tunnel
	//   area 1 section A - city
	//          section B - tunnel
	//
	// MADALINA:
	//
	//   area 0 section A - desert
	//          section B - tunnel
	//   area 1 section A - lake
	//          section B - tunnel

	tilemap_set_user_data(tilemap_edge1[0], map); /* left edge */
	tilemap_set_user_data(tilemap_edge1[1], map);
	tilemap_set_user_data(tilemap_edge1[2], map);
	tilemap_set_user_data(tilemap_edge1[3], map);

	map += 0x80;

	tilemap_set_user_data(tilemap_edge2[0], map); /* right edge */
	tilemap_set_user_data(tilemap_edge2[1], map);
	tilemap_set_user_data(tilemap_edge2[2], map);
	tilemap_set_user_data(tilemap_edge2[3], map);

	tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
}


static int scan_helper(int col, int row, int section)
{
	return section * 0x100 + (~col & 15) * 8 + row;
}


static TILEMAP_MAPPER( scan_mode0 )
{
	return scan_helper(col, row, 0);
}
static TILEMAP_MAPPER( scan_mode1 )
{
	return scan_helper(col, row, 1);
}
static TILEMAP_MAPPER( scan_mode2 )
{
	return scan_helper(col, row, BIT(col, 4) ? 1 : 0);
}
static TILEMAP_MAPPER( scan_mode3 )
{
	return scan_helper(col, row, BIT(col, 4) ? 0 : 1);
}


static TILE_GET_INFO( get_tile_info_BG )
{
	const UINT8* p = param;

	SET_TILE_INFO(1, p[tile_index], BIT(madalien_video_flags, 2) ? 2 : 0, 0);
}


static TILE_GET_INFO( get_tile_info_FG )
{
	SET_TILE_INFO(0, madalien_videoram[tile_index], 0, 0);
}


static VIDEO_START( madalien )
{
	rectangle rect = { 0, 127, 0, 127 };

	static const crtc6845_interface crtc6845_intf =
	{
		0,                /* screen we are acting on */
		PIXEL_CLOCK / 8,  /* the clock of the chip  */
		8,                /* number of pixels per video memory address */
		NULL,             /* before pixel update callback */
		NULL,             /* row update callback */
		NULL,             /* after pixel update callback */
		NULL              /* call back for display state changes */
	};

	crtc6845_config(0, &crtc6845_intf);

	tilemap_fg = tilemap_create(get_tile_info_FG,
		tilemap_scan_cols_flip_x, TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(tilemap_fg, 0);

	tilemap_edge1[0] = tilemap_create(get_tile_info_BG,
		scan_mode0, TILEMAP_TYPE_PEN, 16, 16, 16, 8);
	tilemap_edge1[1] = tilemap_create(get_tile_info_BG,
		scan_mode1, TILEMAP_TYPE_PEN, 16, 16, 16, 8);
	tilemap_edge1[2] = tilemap_create(get_tile_info_BG,
		scan_mode2, TILEMAP_TYPE_PEN, 16, 16, 32, 8);
	tilemap_edge1[3] = tilemap_create(get_tile_info_BG,
		scan_mode3, TILEMAP_TYPE_PEN, 16, 16, 32, 8);

	tilemap_edge2[0] = tilemap_create(get_tile_info_BG,
		scan_mode0, TILEMAP_TYPE_PEN, 16, 16, 16, 8);
	tilemap_edge2[1] = tilemap_create(get_tile_info_BG,
		scan_mode1, TILEMAP_TYPE_PEN, 16, 16, 16, 8);
	tilemap_edge2[2] = tilemap_create(get_tile_info_BG,
		scan_mode2, TILEMAP_TYPE_PEN, 16, 16, 32, 8);
	tilemap_edge2[3] = tilemap_create(get_tile_info_BG,
		scan_mode3, TILEMAP_TYPE_PEN, 16, 16, 32, 8);

	update_edges(0);

	tilemap_set_scrolldy(tilemap_edge2[0], 0, machine->screen[0].height - 256);
	tilemap_set_scrolldy(tilemap_edge2[1], 0, machine->screen[0].height - 256);
	tilemap_set_scrolldy(tilemap_edge2[2], 0, machine->screen[0].height - 256);
	tilemap_set_scrolldy(tilemap_edge2[3], 0, machine->screen[0].height - 256);

	tilemap_set_flip(tilemap_edge2[0], TILEMAP_FLIPY);
	tilemap_set_flip(tilemap_edge2[1], TILEMAP_FLIPY);
	tilemap_set_flip(tilemap_edge2[2], TILEMAP_FLIPY);
	tilemap_set_flip(tilemap_edge2[3], TILEMAP_FLIPY);

	headlight_bitmap = auto_bitmap_alloc(128, 128, BITMAP_FORMAT_INDEXED16);

	drawgfx(headlight_bitmap, machine->gfx[2],
		0, 0, 0, 0, 0x00, 0x00, &rect, TRANSPARENCY_NONE, 0);
	drawgfx(headlight_bitmap, machine->gfx[2],
		0, 0, 0, 1, 0x00, 0x40, &rect, TRANSPARENCY_NONE, 0);

	tmp_bitmap = auto_bitmap_alloc(256, 256, BITMAP_FORMAT_INDEXED16);
}


static VIDEO_UPDATE( madalien )
{
	int i;

	int flip_screen;

	/* draw background */

	rectangle rect = { 0, 255, 0, 255 };

	rectangle clip_edge1 = rect;
	rectangle clip_edge2 = rect;

	int mode = madalien_scroll & 3;

	clip_edge1.max_y = madalien_edge1_pos ^ 0x7f;
	clip_edge2.min_y = madalien_edge2_pos | 0x80;

	fillbitmap(tmp_bitmap, machine->pens[0], &rect);

	tilemap_draw(
		tmp_bitmap, &clip_edge1, tilemap_edge1[mode], 0, 0);
	tilemap_draw(
		tmp_bitmap, &clip_edge2, tilemap_edge2[mode], 0, 0);

	/* draw foreground */

	for (i = 0; i < 256; i++)
	{
		decodechar(machine->gfx[0], i, madalien_charram);
	}

	tilemap_mark_all_tiles_dirty(tilemap_fg);

	tilemap_draw(tmp_bitmap, &rect, tilemap_fg, 0, 0);

	/* highlight section A (outside of tunnels) */

	if (mode != 1)
	{
		int x;
		int y;

		int _L = 0;
		int _T = 0;
		int _R = 256;
		int _B = 256;

		if (mode == 2)
		{
			if (_L < (madalien_scroll & 0xfc))
				_L = (madalien_scroll & 0xfc);
		}
		if (mode == 3)
		{
			if (_R > (madalien_scroll & 0xfc))
				_R = (madalien_scroll & 0xfc);
		}

		for (y = _T; y < _B; y++)
		{
			for (x = _L; x < _R; x++)
			{
				*BITMAP_ADDR16(tmp_bitmap, y, x) |= 8;
			}
		}
	}

	/* draw car headlight */

	if (BIT(madalien_video_flags, 0))
	{
		int x;
		int y;

		for (y = 0; y < 128; y++)
		{
			int ypos = (y - madalien_headlight_pos) & 0xff;

			for (x = 0; x < 128; x++)
			{
				if (*BITMAP_ADDR16(headlight_bitmap, y, x) != 0)
				{
					*BITMAP_ADDR16(tmp_bitmap, ypos, x) |= 8;
				}
			}
		}
	};

	/* flip screen if necessary */

	flip_screen = 0;

	if (BIT(readinputportbytag("DIP"), 6)) /* cocktail DIP */
	{
		if (BIT(madalien_screen_control, 0))
		{
			flip_screen = 1;
		}
	}

	copybitmap(bitmap, tmp_bitmap, flip_screen, flip_screen,
		0, 0, cliprect, TRANSPARENCY_NONE, 0);

	return 0;
}


static INTERRUPT_GEN( madalien_interrupt )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_NMI,
		(readinputportbytag("PLAYER2") & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


static READ8_HANDLER( madalien_shift_lo_r )
{
	const UINT8* table = memory_region(REGION_USER1);

	return table[256 * madalien_shift_count + madalien_shift_data];
}


static READ8_HANDLER( madalien_shift_hi_r )
{
	UINT8 result = 0;

	UINT8 flip = 0;

	if (BIT(madalien_shift_data, 0)) flip |= 1 << 7;
	if (BIT(madalien_shift_data, 1)) flip |= 1 << 6;
	if (BIT(madalien_shift_data, 2)) flip |= 1 << 5;
	if (BIT(madalien_shift_data, 3)) flip |= 1 << 4;
	if (BIT(madalien_shift_data, 4)) flip |= 1 << 3;
	if (BIT(madalien_shift_data, 5)) flip |= 1 << 2;
	if (BIT(madalien_shift_data, 6)) flip |= 1 << 1;
	if (BIT(madalien_shift_data, 7)) flip |= 1 << 0;

	{
		const UINT8* table = memory_region(REGION_USER1);

		UINT8 lookup = table[256 * (madalien_shift_count ^ 7) + flip];

		if (BIT(lookup, 0)) result |= 1 << 6;
		if (BIT(lookup, 1)) result |= 1 << 5;
		if (BIT(lookup, 2)) result |= 1 << 4;
		if (BIT(lookup, 3)) result |= 1 << 3;
		if (BIT(lookup, 4)) result |= 1 << 2;
		if (BIT(lookup, 5)) result |= 1 << 1;
		if (BIT(lookup, 6)) result |= 1 << 0;
	}

	return result;
}


static WRITE8_HANDLER(madalien_video_flags_w)
{
	data &= 15;

	if (madalien_video_flags != data)
	{
		update_edges(data >> 3);
	}

	madalien_video_flags = data;
}


static WRITE8_HANDLER( madalien_headlight_pos_w )
{
	madalien_headlight_pos = data;
}


static WRITE8_HANDLER( madalien_scroll_w )
{
	int i;

	// bits #0 and #1 define scrolling mode
	//
	// mode 0 - cycle over map section A
	// mode 1 - cycle over map section B
	//
	// mode 2 - transition from B to A
	// mode 3 - transition from A to B

	for (i = 0; i < 4; i++)
	{
		tilemap_set_scrollx(
			tilemap_edge1[i], 0, -(data & 0xfc));
		tilemap_set_scrollx(
			tilemap_edge2[i], 0, -(data & 0xfc));
	}

	madalien_scroll = data;
}


static WRITE8_HANDLER( madalien_edge1_pos_w )
{
	madalien_edge1_pos = data & 0x7f;

	tilemap_set_scrolly(tilemap_edge1[0], 0, madalien_edge1_pos);
	tilemap_set_scrolly(tilemap_edge1[1], 0, madalien_edge1_pos);
	tilemap_set_scrolly(tilemap_edge1[2], 0, madalien_edge1_pos);
	tilemap_set_scrolly(tilemap_edge1[3], 0, madalien_edge1_pos);
}


static WRITE8_HANDLER( madalien_edge2_pos_w )
{
	madalien_edge2_pos = data & 0x7f;

	tilemap_set_scrolly(tilemap_edge2[0], 0, madalien_edge2_pos);
	tilemap_set_scrolly(tilemap_edge2[1], 0, madalien_edge2_pos);
	tilemap_set_scrolly(tilemap_edge2[2], 0, madalien_edge2_pos);
	tilemap_set_scrolly(tilemap_edge2[3], 0, madalien_edge2_pos);
}


static WRITE8_HANDLER( madalien_shift_data_w )
{
	madalien_shift_data = data;
}
static WRITE8_HANDLER( madalien_shift_count_w )
{
	madalien_shift_count = data & 7;
}


static WRITE8_HANDLER( madalien_screen_control_w )
{
	/* bit #0 is set during player 2's turn, bit #3 is set during CRTC initialization */

	madalien_screen_control = data & 15;
}


static WRITE8_HANDLER( madalien_output_w )
{
	/* output latch, eight output bits, none connected */
}


static WRITE8_HANDLER( madalien_sound_command_w )
{
	cpunum_set_input_line(Machine, 1, 0, ASSERT_LINE);

	soundlatch_w(offset, data);
}


static READ8_HANDLER(madalien_sound_command_r )
{
	cpunum_set_input_line(Machine, 1, 0, CLEAR_LINE);

	return soundlatch_r(offset);
}


static WRITE8_HANDLER( madalien_portA_w )
{
	/* not emulated - amplification? */
}
static WRITE8_HANDLER( madalien_portB_w )
{
	/* not emulated - motor sound? */
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM

	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_BASE(&madalien_videoram)
	AM_RANGE(0x6400, 0x67ff) AM_RAM
	AM_RANGE(0x6800, 0x7fff) AM_RAM AM_BASE(&madalien_charram)

	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x0ff0) AM_WRITE(crtc6845_address_w)
	AM_RANGE(0x8001, 0x8001) AM_MIRROR(0x0ff0) AM_READWRITE(crtc6845_register_r, crtc6845_register_w)
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(0x0ff0) AM_WRITE(madalien_screen_control_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(0x0ff0) AM_WRITE(madalien_output_w)
	AM_RANGE(0x8006, 0x8006) AM_MIRROR(0x0ff0) AM_READWRITE(soundlatch2_r, madalien_sound_command_w)
	AM_RANGE(0x8008, 0x8008) AM_MIRROR(0x07f0) AM_READWRITE(madalien_shift_lo_r, madalien_shift_count_w)
	AM_RANGE(0x8009, 0x8009) AM_MIRROR(0x07f0) AM_READWRITE(madalien_shift_hi_r, madalien_shift_data_w)
	AM_RANGE(0x800b, 0x800b) AM_MIRROR(0x07f0) AM_WRITE(madalien_video_flags_w)
	AM_RANGE(0x800c, 0x800c) AM_MIRROR(0x07f0) AM_WRITE(madalien_headlight_pos_w)
	AM_RANGE(0x800d, 0x800d) AM_MIRROR(0x07f0) AM_WRITE(madalien_edge1_pos_w)
	AM_RANGE(0x800e, 0x800e) AM_MIRROR(0x07f0) AM_WRITE(madalien_edge2_pos_w)
	AM_RANGE(0x800f, 0x800f) AM_MIRROR(0x07f0) AM_WRITE(madalien_scroll_w)

	AM_RANGE(0x9000, 0x9000) AM_MIRROR(0x0ff0) AM_READ(input_port_0_r)
	AM_RANGE(0x9001, 0x9001) AM_MIRROR(0x0ff0) AM_READ(input_port_1_r)
	AM_RANGE(0x9002, 0x9002) AM_MIRROR(0x0ff0) AM_READ(input_port_2_r)

	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x1ffc) AM_RAM /* unknown device in an epoxy block, might be tilt detection */
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1ffc) AM_READ(madalien_sound_command_r)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1ffc) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8001, 0x8001) AM_MIRROR(0x1ffc) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x8002, 0x8002) AM_MIRROR(0x1ffc) AM_WRITE(soundlatch2_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( madalien )
	PORT_START_TAG("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DIP")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage )) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(	0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life )) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "7000" )
	PORT_DIPSETTING(    0x30, "never" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START_TAG("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) /* or service coin */
INPUT_PORTS_END



static const UINT32 headlight_xoffset[] =
{
	STEP8(0x78, 1),
	STEP8(0x70, 1),
	STEP8(0x68, 1),
	STEP8(0x60, 1),
	STEP8(0x58, 1),
	STEP8(0x50, 1),
	STEP8(0x48, 1),
	STEP8(0x40, 1),
	STEP8(0x38, 1),
	STEP8(0x30, 1),
	STEP8(0x28, 1),
	STEP8(0x20, 1),
	STEP8(0x18, 1),
	STEP8(0x10, 1),
	STEP8(0x08, 1),
	STEP8(0x00, 1),
};

static const UINT32 headlight_yoffset[] =
{
	STEP32(0x0000, 0x80), STEP32(0x1000, 0x80)
};

static const gfx_layout headlightlayout =
{
	128, 64,
	1,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x2000,
	headlight_xoffset,
	headlight_yoffset
};

static const gfx_layout charlayout =
{
	8, 8,
	256,
	3,
	{ 2*0x4000, 1*0x4000, 0*0x4000 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	64
};

static const gfx_layout tilelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{
		0x30*8+4, 0x30*8+5, 0x30*8+6, 0x30*8+7,
		0x20*8+4, 0x20*8+5, 0x20*8+6, 0x20*8+7,
		0x10*8+4, 0x10*8+5, 0x10*8+6, 0x10*8+7,
		0x00*8+4, 0x00*8+5, 0x00*8+6, 0x00*8+7
	},
	{ STEP16(0, 8) },
	0x200
};


static GFXDECODE_START( madalien )
	GFXDECODE_ENTRY( 0, 0, charlayout, 32, 2 ) /* foreground characters, stored in RAM */
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout, 0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, headlightlayout, 0, 1 )
GFXDECODE_END


static const struct AY8910interface ay8910_interface =
{
	NULL,
	NULL,
	madalien_portA_w,
	madalien_portB_w
};


static MACHINE_DRIVER_START( madalien )

	/* main CPU */
	MDRV_CPU_ADD_TAG("main", M6502, MAIN_CLOCK / 8)    /* 1324kHz */
	MDRV_CPU_PROGRAM_MAP(main_map, 0)
	MDRV_CPU_VBLANK_INT(madalien_interrupt, 1)

	/* audio CPU */
	MDRV_CPU_ADD_TAG("sound", M6502, SOUND_CLOCK / 8)   /* 512kHz */
	MDRV_CPU_PROGRAM_MAP(sound_map, 0)
	MDRV_CPU_PERIODIC_INT(nmi_line_pulse, 800)    /* unknown due to incomplete schematics */

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 336, 0, 256, 288, 0, 256)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_GFXDECODE(madalien)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(48)
	MDRV_PALETTE_INIT(madalien)
	MDRV_VIDEO_START(madalien)
	MDRV_VIDEO_UPDATE(madalien)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, SOUND_CLOCK / 4)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_DRIVER_END


ROM_START( madalien )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )                   /* main CPU */
	ROM_LOAD( "m7.3f",	0xc000, 0x0800, CRC(4d12f89d) SHA1(e155f9135bc2bea56e211052f2b74d25e76308c8) )
	ROM_LOAD( "m6.3h",	0xc800, 0x0800, CRC(1bc4a57b) SHA1(02252b868d0c07c0a18240e9d831c303cdcfa9a6) )
	ROM_LOAD( "m5.3k",	0xd000, 0x0800, CRC(8db99572) SHA1(f8cf22f8c134b47756b7f02c5ca0217100466744) )
	ROM_LOAD( "m4.3l",	0xd800, 0x0800, CRC(fba671af) SHA1(dd74bd357c82d525948d836a7f860bbb3182c825) )
	ROM_LOAD( "m3.4f",	0xe000, 0x0800, CRC(1aad640d) SHA1(9ace7d2c5ef9e789c2b8cc65420b19ce72cd95fa) )
	ROM_LOAD( "m2.4h",	0xe800, 0x0800, CRC(cbd533a0) SHA1(d3be81fb9ba40e30e5ff0171efd656b11dd20f2b) )
	ROM_LOAD( "m1.4k",	0xf000, 0x0800, CRC(ad654b1d) SHA1(f8b365dae3801e97e04a10018a790d3bdb5d9439) )
	ROM_LOAD( "m0.4l",	0xf800, 0x0800, CRC(cf7aa787) SHA1(f852cc806ecc582661582326747974a14f50174a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )                   /* audio CPU */
	ROM_LOAD( "m8", 0xf800, 0x0400, CRC(cfd19dab) SHA1(566dc84ffe9bcaeb112250a9e1882bf62f47b579) )
	ROM_LOAD( "m9", 0xfc00, 0x0400, CRC(48f30f24) SHA1(9c0bf6e43b143d6af1ebb9dad2bdc2b53eb2e48e) )

	ROM_REGION( 0x0c00, REGION_GFX1, ROMREGION_DISPOSE )    /* background tiles */
	ROM_LOAD( "mc.3k", 0x0000, 0x0400, CRC(2daadfb7) SHA1(8be084a39b256e538fd57111e92d47115cb142cd) )
	ROM_LOAD( "md.3l", 0x0400, 0x0400, CRC(3ee1287a) SHA1(33bc59a8d09d22f3db80f881c2f37aa788718138) )
	ROM_LOAD( "me.3m", 0x0800, 0x0400, CRC(45a5c201) SHA1(ac600afeabf494634c3189d8e96644bd0deb45f3) )

	ROM_REGION( 0x0400, REGION_GFX2, 0 )                    /* background tile map */
	ROM_LOAD( "mf.4h", 0x0000, 0x0400, CRC(e9cba773) SHA1(356c7edb1b412a9e04f0747e780c945af8791c55) )

	ROM_REGION( 0x0400, REGION_GFX3, ROMREGION_DISPOSE )    /* headlight */
	ROM_LOAD( "ma.2b", 0x0000, 0x0400, CRC(aab16446) SHA1(d2342627cc2766004343f27515d8a7989d5fe932) )

	ROM_REGION( 0x0800, REGION_USER1, 0 )                   /* shifting table */
	ROM_LOAD( "mb.5c", 0x0000, 0x0800, CRC(cb801e49) SHA1(7444c4af7cf07e5fdc54044d62ea4fcb201b2b8b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )                   /* color PROM */
	ROM_LOAD( "mg.7f",	0x0000, 0x0020, CRC(3395b31f) SHA1(26235fb448a4180c58f0887e53a29c17857b3b34) )
ROM_END


ROM_START( madalina )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )                   /* main CPU */
	ROM_LOAD( "2716.4c", 0xb000, 0x0800, CRC(90be68af) SHA1(472ccfd2e04d6d49be47d919cba0c55d850b2887) )
	ROM_LOAD( "2716.4e", 0xb800, 0x0800, CRC(aba10cbb) SHA1(6ca213ded8ed7f4f310ab5ae25220cf867dd1d00) )
	ROM_LOAD( "2716.3f", 0xc000, 0x0800, CRC(c3af484c) SHA1(c3667526d3b5aeee68823f92826053e657512851) )
	ROM_LOAD( "2716.3h", 0xc800, 0x0800, CRC(78ca5a87) SHA1(729d69ee63c710241a098471e9769063dfe8ef1e) )
	ROM_LOAD( "2716.3k", 0xd000, 0x0800, CRC(070e81ea) SHA1(006831f4bf289812e4e87a3ece7885e8b901f2f5) )
	ROM_LOAD( "2716.3l", 0xd800, 0x0800, CRC(98225cb0) SHA1(ca74f5e33fa9116215b03abadd5d09840c04fb0b) )
	ROM_LOAD( "2716.4f", 0xe000, 0x0800, CRC(52fea0fc) SHA1(443fd859daf4279d5976256a4b1c970b520661a2) )
	ROM_LOAD( "2716.4h", 0xe800, 0x0800, CRC(dba6c4f6) SHA1(51f815fc7eb99a05eee6204de2d4cad1734adc52) )
	ROM_LOAD( "2716.4k", 0xf000, 0x0800, CRC(06991af6) SHA1(19112306529721222b6e1c07920348c263d8b8aa) )
	ROM_LOAD( "2716.4l", 0xf800, 0x0800, CRC(57752b47) SHA1(a34d3150ea9082889154042dbea3386f71322a78) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )                   /* audio CPU */
	ROM_LOAD( "8_2708.4d", 0xf800, 0x0400, CRC(46162e7e) SHA1(7ed85f4a9ac58d6d9bafba0c843a16c269656563) )
	ROM_LOAD( "9_2708.3d", 0xfc00, 0x0400, CRC(4175f5c4) SHA1(45cae8a1fcfd34b91c63cc7e544a32922da14f16) )

	ROM_REGION( 0x0c00, REGION_GFX1, ROMREGION_DISPOSE )    /* background tiles */
	ROM_LOAD( "mc-1.3k", 0x0000, 0x0400, NO_DUMP )
	ROM_LOAD( "me-1.3l", 0x0400, 0x0400, CRC(7328a425) SHA1(327adc8b0e25d93f1ae98a44c26d0aaaac1b1a9c) )
	ROM_LOAD( "md-1.3m", 0x0800, 0x0400, CRC(b5329929) SHA1(86890e1b7cc8cb31fc0dcbc2d3cff02e4cf95619) )

	ROM_REGION( 0x0400, REGION_GFX2, 0 )                    /* background tile map */
	ROM_LOAD( "mf-1.4h", 0x0000, 0x0400, CRC(9b04c446) SHA1(918013f3c0244ab6a670b9d1b6b642298e2c5ab8) )

	ROM_REGION( 0x0400, REGION_GFX3, ROMREGION_DISPOSE )    /* headlight */
	ROM_LOAD( "ma.2b", 0x0000, 0x0400, CRC(aab16446) SHA1(d2342627cc2766004343f27515d8a7989d5fe932) )

	ROM_REGION( 0x0800, REGION_USER1, 0 )                   /* shifting table */
	ROM_LOAD( "mb.5c", 0x0000, 0x0800, CRC(cb801e49) SHA1(7444c4af7cf07e5fdc54044d62ea4fcb201b2b8b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )                   /* color PROM */
	ROM_LOAD( "mg-1.7f", 0x0000, 0x0020, CRC(e622396a) SHA1(8972704bd25fed462e25c453771cc5ca4fc74034) )
ROM_END


static DRIVER_INIT( madalien )
{
	state_save_register_global(madalien_video_flags);
	state_save_register_global(madalien_screen_control);
	state_save_register_global(madalien_scroll);
	state_save_register_global(madalien_edge1_pos);
	state_save_register_global(madalien_edge2_pos);
	state_save_register_global(madalien_headlight_pos);
	state_save_register_global(madalien_shift_count);
	state_save_register_global(madalien_shift_data);
}


/*          rom       parent     machine   inp       init */
GAME( 1980, madalien, 0,         madalien, madalien, madalien, ROT270, "Data East Corporation", "Mad Alien", GAME_IMPERFECT_SOUND )
GAME( 1980, madalina, madalien,  madalien, madalien, madalien, ROT270, "Data East Corporation", "Mad Alien (Highway Chase)", GAME_IMPERFECT_SOUND )
