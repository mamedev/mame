/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "driver.h"
#include "video/mc6845.h"
#include "includes/madalien.h"


#define PIXEL_CLOCK (MADALIEN_MAIN_CLOCK / 2)


UINT8 *madalien_videoram;
UINT8 *madalien_charram;

UINT8 *madalien_video_flags;
UINT8 *madalien_video_control;	/* bit #0 is set during player 2's turn, bit #3 is set during CRTC initialization */
UINT8 *madalien_scroll;
UINT8 *madalien_edge1_pos;
UINT8 *madalien_edge2_pos;
UINT8 *madalien_headlight_pos;

static tilemap_t *tilemap_fg;

static tilemap_t *tilemap_edge1[4];
static tilemap_t *tilemap_edge2[4];

static bitmap_t *headlight_bitmap;


static PALETTE_INIT( madalien )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	for (i = 0; i < 0x20; i++)
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

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 0x10; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	for (i = 0x10; i < 0x20; i++)
	{
		UINT8 ctabentry = i - 0x10;

		if (BIT((i - 0x10), 1))
			ctabentry = ctabentry ^ 0x06;

		if (BIT((i - 0x10), 2))
			ctabentry = ctabentry ^ 0x06;

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	for (i = 0x20; i < 0x30; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x20) | 0x10);
}


INLINE int scan_helper(int col, int row, int section)
{
	return (section << 8) | ((~col & 0x0f) << 3) | row;
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


static TILE_GET_INFO( get_tile_info_BG_1 )
{
	UINT8 *map = memory_region(machine, "user1") + ((*madalien_video_flags & 0x08) << 6);

	SET_TILE_INFO(1, map[tile_index], BIT(*madalien_video_flags, 2) ? 2 : 0, 0);
}


static TILE_GET_INFO( get_tile_info_BG_2 )
{
	UINT8 *map = memory_region(machine, "user1") + ((*madalien_video_flags & 0x08) << 6) + 0x80;

	SET_TILE_INFO(1, map[tile_index], BIT(*madalien_video_flags, 2) ? 2 : 0, 0);
}


static TILE_GET_INFO( get_tile_info_FG )
{
	SET_TILE_INFO(0, madalien_videoram[tile_index], 0, 0);
}

WRITE8_HANDLER( madalien_videoram_w )
{
	madalien_videoram[offset] = data;
	tilemap_mark_tile_dirty(tilemap_fg, offset);
}


static VIDEO_START( madalien )
{
	int i;

	static const tilemap_mapper_func scan_functions[4] =
	{
		scan_mode0, scan_mode1, scan_mode2, scan_mode3
	};

	static const int tilemap_cols[4] =
	{
		16, 16, 32, 32
	};

	tilemap_fg = tilemap_create(machine, get_tile_info_FG, tilemap_scan_cols_flip_x, 8, 8, 32, 32);
	tilemap_set_transparent_pen(tilemap_fg, 0);
	tilemap_set_scrolldx(tilemap_fg, 0, 0x50);
	tilemap_set_scrolldy(tilemap_fg, 0, 0x20);

	for (i = 0; i < 4; i++)
	{
		tilemap_edge1[i] = tilemap_create(machine, get_tile_info_BG_1, scan_functions[i], 16, 16, tilemap_cols[i], 8);
		tilemap_set_scrolldx(tilemap_edge1[i], 0, 0x50);
		tilemap_set_scrolldy(tilemap_edge1[i], 0, 0x20);

		tilemap_edge2[i] = tilemap_create(machine, get_tile_info_BG_2, scan_functions[i], 16, 16, tilemap_cols[i], 8);
		tilemap_set_scrolldx(tilemap_edge2[i], 0, 0x50);
		tilemap_set_scrolldy(tilemap_edge2[i], 0, video_screen_get_height(machine->primary_screen) - 256);
	}

	headlight_bitmap = auto_bitmap_alloc(machine, 128, 128, BITMAP_FORMAT_INDEXED16);

	gfx_element_set_source(machine->gfx[0], madalien_charram);

	drawgfx_opaque(headlight_bitmap, NULL, machine->gfx[2], 0, 0, 0, 0, 0x00, 0x00);
	drawgfx_opaque(headlight_bitmap, NULL, machine->gfx[2], 0, 0, 0, 1, 0x00, 0x40);
}


static void draw_edges(bitmap_t *bitmap, const rectangle *cliprect, int flip, int scroll_mode)
{
	rectangle clip_edge1;
	rectangle clip_edge2;

	clip_edge1 = *cliprect;
	clip_edge2 = *cliprect;

	if (flip)
	{
		clip_edge1.min_y = *madalien_edge1_pos | 0x80;
		clip_edge2.max_y = (*madalien_edge2_pos & 0x7f) ^ 0x7f;
	}
	else
	{
		clip_edge1.max_y = (*madalien_edge1_pos & 0x7f) ^ 0x7f;
		clip_edge2.min_y = *madalien_edge2_pos | 0x80;
	}

	sect_rect(&clip_edge1, cliprect);
	sect_rect(&clip_edge2, cliprect);

	tilemap_mark_all_tiles_dirty(tilemap_edge1[scroll_mode]);
	tilemap_mark_all_tiles_dirty(tilemap_edge2[scroll_mode]);

	tilemap_set_flip(tilemap_edge1[scroll_mode], flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_set_scrollx(tilemap_edge1[scroll_mode], 0, -(*madalien_scroll & 0xfc));
	tilemap_set_scrolly(tilemap_edge1[scroll_mode], 0, *madalien_edge1_pos & 0x7f);

	tilemap_set_flip(tilemap_edge2[scroll_mode], flip ? TILEMAP_FLIPX : TILEMAP_FLIPY);
	tilemap_set_scrollx(tilemap_edge2[scroll_mode], 0, -(*madalien_scroll & 0xfc));
	tilemap_set_scrolly(tilemap_edge2[scroll_mode], 0, *madalien_edge2_pos & 0x7f);

	tilemap_draw(bitmap, &clip_edge1, tilemap_edge1[scroll_mode], 0, 0);
	tilemap_draw(bitmap, &clip_edge2, tilemap_edge2[scroll_mode], 0, 0);
}


static void draw_headlight(bitmap_t *bitmap, const rectangle *cliprect, int flip)
{
	if (BIT(*madalien_video_flags, 0))
	{
		UINT8 y;

		for (y = 0; y < 0x80; y++)
		{
			UINT8 x;
			UINT8 hy = y - *madalien_headlight_pos;

			if (flip)
				hy = ~hy;

			if ((hy < cliprect->min_y) || (hy > cliprect->max_y))
				continue;

			for (x = 0; x < 0x80; x++)
			{
				UINT8 hx = x;

				if (flip)
					hx = ~hx;

				if ((hx < cliprect->min_x) || (hx > cliprect->max_x))
					continue;

				if (*BITMAP_ADDR16(headlight_bitmap, y, x) != 0)
					*BITMAP_ADDR16(bitmap, hy, hx) |= 8;
			}
		}
	}
}


static void draw_foreground(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int flip)
{
	tilemap_set_flip(tilemap_fg, flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap, cliprect, tilemap_fg, 0, 0);
}


WRITE8_HANDLER( madalien_charram_w )
{
	madalien_charram[offset] = data;
	gfx_element_mark_dirty(space->machine->gfx[0], (offset/8) & 0xff);
}


static VIDEO_UPDATE( madalien )
{
	int flip = BIT(input_port_read(screen->machine, "DSW"), 6) && BIT(*madalien_video_control, 0);

	// bits #0 and #1 define scrolling mode
	//
	// mode 0 - cycle over map section A
	// mode 1 - cycle over map section B
	//
	// mode 2 - transition from B to A
	// mode 3 - transition from A to B
	int scroll_mode = *madalien_scroll & 3;

	bitmap_fill(bitmap, cliprect, 0);
	draw_edges(bitmap, cliprect, flip, scroll_mode);
	draw_foreground(screen->machine, bitmap, cliprect, flip);

	/* highlight section A (outside of tunnels) */

	if (scroll_mode != 1)
	{
		int x;
		int y;

		int min_x = 0;
		int max_x = 0xff;

		if (scroll_mode == 2)
			min_x = (*madalien_scroll & 0xfc);

		if (scroll_mode == 3)
			max_x = (*madalien_scroll & 0xfc) - 1;

		if (flip)
		{
			int max_x_save = max_x;
			max_x = 0xff - min_x;
			min_x = 0xff - max_x_save;
		}

		for (y = cliprect->min_y; y <= cliprect->max_y ; y++)
			for (x = min_x; x <= max_x; x++)
				if ((x >= cliprect->min_x) && (x <= cliprect->max_x))
					*BITMAP_ADDR16(bitmap, y, x) |= 8;
	}

	draw_headlight(bitmap, cliprect, flip);

	return 0;
}


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
	GFXDECODE_ENTRY( NULL,   0, charlayout,     0x20, 2 ) /* foreground characters, stored in RAM */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,        0, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, headlightlayout,   0, 1 )
GFXDECODE_END


static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


MACHINE_DRIVER_START( madalien_video )
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 336, 0, 256, 288, 0, 256)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_GFXDECODE(madalien)
	MDRV_PALETTE_LENGTH(0x30)
	MDRV_PALETTE_INIT(madalien)
	MDRV_VIDEO_START(madalien)
	MDRV_VIDEO_UPDATE(madalien)

	MDRV_MC6845_ADD("crtc", MC6845, PIXEL_CLOCK / 8, mc6845_intf)
MACHINE_DRIVER_END
