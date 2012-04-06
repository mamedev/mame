/***************************************************************************

                            -= Clash Road =-

                    driver by   Luca Elia (l.elia@tin.it)

    [ 2 Horizontally Scrolling Layers ]

        Size :  512 x 256
        Tiles:  16 x 16 x 4.

        These 2 layers share the same graphics and X scroll value.
        The tile codes are stuffed together in memory too: first one
        layer's row, then the other's (and so on for all the rows).

    [ 1 Fixed Layer ]

        Size :  (256 + 32) x 256
        Tiles:  8 x 8 x 4.

        This is like a 32x32 tilemap, but the top and bottom rows (that
        fall outside the visible area) are used to widen the tilemap
        horizontally, adding 2 vertical columns both sides.

        The result is a 36x28 visible tilemap.

    [ 64? sprites ]

        Sprites are 16 x 16 x 4.

***************************************************************************/

#include "emu.h"
#include "includes/clshroad.h"


WRITE8_MEMBER(clshroad_state::clshroad_flipscreen_w)
{
	flip_screen_set(machine(),  data & 1 );
}


PALETTE_INIT( clshroad )
{
	int i;
	for (i = 0;i < 256;i++)
		palette_set_color_rgb(machine,i,	pal4bit(color_prom[i + 256 * 0]),
								        pal4bit(color_prom[i + 256 * 1]),
								        pal4bit(color_prom[i + 256 * 2]));
}

PALETTE_INIT( firebatl )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (i = 0; i < 0x200; i++)
		colortable_entry_set_value(machine.colortable, i, i & 0xff);

	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry = ((color_prom[(i - 0x200) + 0x000] & 0x0f) << 4) |
						   (color_prom[(i - 0x200) + 0x100] & 0x0f);
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}



/***************************************************************************

                        Callbacks for the TileMap code

***************************************************************************/

/***************************************************************************

                          Layers 0 Tiles Format

Offset:

    00-3f:  Even bytes: Codes   Odd bytes: Colors   <- Layer B First Row
    40-7f:  Even bytes: Codes   Odd bytes: Colors   <- Layer A First Row
    ..                                      <- 2nd Row
    ..                                      <- 3rd Row
    etc.

***************************************************************************/

static TILE_GET_INFO( get_tile_info_0a )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code	=	state->m_vram_0[ tile_index * 2 + 0x40 ];
//  color   =   state->m_vram_0[ tile_index * 2 + 0x41 ];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

static TILE_GET_INFO( get_tile_info_0b )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code	=	state->m_vram_0[ tile_index * 2 + 0x00 ];
//  color   =   state->m_vram_0[ tile_index * 2 + 0x01 ];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

WRITE8_MEMBER(clshroad_state::clshroad_vram_0_w)
{
	int tile_index = offset / 2;
	int tile = (tile_index & 0x1f) + (tile_index & ~0x3f)/2;
	m_vram_0[offset] = data;
	if (tile_index & 0x20)	m_tilemap_0a->mark_tile_dirty(tile);
	else					m_tilemap_0b->mark_tile_dirty(tile);
}

/***************************************************************************

                          Layer 1 Tiles Format

Offset:

    000-3ff     Code
    400-7ff     7654----    Code (High bits)
                ----3210    Color

    This is like a 32x32 tilemap, but the top and bottom rows (that
    fall outside the visible area) are used to widen the tilemap
    horizontally, adding 2 vertical columns both sides.

    The result is a 36x28 visible tilemap.

***************************************************************************/

/* logical (col,row) -> memory offset */
static TILEMAP_MAPPER( tilemap_scan_rows_extra )
{
	// The leftmost columns come from the bottom rows
	if (col <= 0x01)	return row + (col + 0x1e) * 0x20;
	// The rightmost columns come from the top rows
	if (col >= 0x22)	return row + (col - 0x22) * 0x20;

	// These are not visible, but *must* be mapped to other tiles than
	// those used by the leftmost and rightmost columns (tilemap "bug"?)
	if (row <= 0x01)	return 0;
	if (row >= 0x1e)	return 0;

	// "normal" layout for the rest.
	return (col-2) + row * 0x20;
}

static TILE_GET_INFO( get_tile_info_fb1 )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	UINT8 code	=	state->m_vram_1[ tile_index + 0x000 ];
	UINT8 color	=	state->m_vram_1[ tile_index + 0x400 ] & 0x3f;
	tileinfo.group = color;
	SET_TILE_INFO(
			2,
			code,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	UINT8 code	=	state->m_vram_1[ tile_index + 0x000 ];
	UINT8 color	=	state->m_vram_1[ tile_index + 0x400 ];
	SET_TILE_INFO(
			2,
			code + ((color & 0xf0)<<4),
			color & 0x0f,
			0);
}

WRITE8_MEMBER(clshroad_state::clshroad_vram_1_w)
{
	m_vram_1[offset] = data;
	m_tilemap_1->mark_tile_dirty(offset % 0x400);
}


VIDEO_START( firebatl )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	/* These 2 use the graphics and scroll value */
	state->m_tilemap_0a = tilemap_create(machine, get_tile_info_0a,tilemap_scan_rows,16,16,0x20,0x10);
	state->m_tilemap_0b = tilemap_create(machine, get_tile_info_0b,tilemap_scan_rows,16,16,0x20,0x10);
	/* Text (No scrolling) */
	state->m_tilemap_1  = tilemap_create(machine, get_tile_info_fb1,tilemap_scan_rows_extra,8,8,0x24,0x20);

	state->m_tilemap_0a->set_scroll_rows(1);
	state->m_tilemap_0b->set_scroll_rows(1);
	state->m_tilemap_1->set_scroll_rows(1);

	state->m_tilemap_0a->set_scroll_cols(1);
	state->m_tilemap_0b->set_scroll_cols(1);
	state->m_tilemap_1->set_scroll_cols(1);

	state->m_tilemap_0a->set_scrolldx(-0x30, -0xb5);
	state->m_tilemap_0b->set_scrolldx(-0x30, -0xb5);

	state->m_tilemap_0b->set_transparent_pen(0 );
	colortable_configure_tilemap_groups(machine.colortable, state->m_tilemap_1, machine.gfx[2], 0x0f);
}

VIDEO_START( clshroad )
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	/* These 2 use the graphics and scroll value */
	state->m_tilemap_0a = tilemap_create(machine, get_tile_info_0a,tilemap_scan_rows,16,16,0x20,0x10);
	state->m_tilemap_0b = tilemap_create(machine, get_tile_info_0b,tilemap_scan_rows,16,16,0x20,0x10);
	/* Text (No scrolling) */
	state->m_tilemap_1  = tilemap_create(machine, get_tile_info_1,tilemap_scan_rows_extra,8,8,0x24,0x20);

	state->m_tilemap_0a->set_scroll_rows(1);
	state->m_tilemap_0b->set_scroll_rows(1);
	state->m_tilemap_1->set_scroll_rows(1);

	state->m_tilemap_0a->set_scroll_cols(1);
	state->m_tilemap_0b->set_scroll_cols(1);
	state->m_tilemap_1->set_scroll_cols(1);

	state->m_tilemap_0a->set_scrolldx(-0x30, -0xb5);
	state->m_tilemap_0b->set_scrolldx(-0x30, -0xb5);

	state->m_tilemap_0b->set_transparent_pen(0x0f );
	state->m_tilemap_1->set_transparent_pen(0x0f );
}


/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

    0

    1                   Y (Bottom-up)

    2       765432--
            ------10    Code (high bits)

    3       76------
            --543210    Code (low bits)

    4

    5                   X (low bits)

    6                   X (High bits)

    7       7654----
            ----3210    Color

- Sprite flipping ?

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	clshroad_state *state = machine.driver_data<clshroad_state>();
	UINT8 *spriteram = state->m_spriteram;
	int i;

	for (i = 0; i < state->m_spriteram_size ; i += 8)
	{
		int y		=	 240 - spriteram[i+1];
		int code	=	(spriteram[i+3] & 0x3f) + (spriteram[i+2] << 6);
		int x		=	 spriteram[i+5]         + (spriteram[i+6] << 8);
		int attr	=	 spriteram[i+7];

		int flipx	=	0;
		int flipy	=	0;

		x -= 0x4a/2;
		if (flip_screen_get(machine))
		{
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				attr & 0x0f,
				flipx,flipy,
				x,y,15 );
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE_IND16( clshroad )
{
	clshroad_state *state = screen.machine().driver_data<clshroad_state>();
	int scrollx  = state->m_vregs[ 0 ] + (state->m_vregs[ 1 ] << 8);
//  int priority = state->m_vregs[ 2 ];

	/* Only horizontal scrolling (these 2 layers use the same value) */
	state->m_tilemap_0a->set_scrollx(0, scrollx);
	state->m_tilemap_0b->set_scrollx(0, scrollx);

	state->m_tilemap_0a->draw(bitmap, cliprect, 0,0);	// Opaque
	state->m_tilemap_0b->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect);
	state->m_tilemap_1->draw(bitmap, cliprect, 0,0);
	return 0;
}
