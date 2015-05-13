// license:BSD-3-Clause
// copyright-holders:Luca Elia
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


WRITE8_MEMBER(clshroad_state::flipscreen_w)
{
	flip_screen_set(data & 1 );
}


PALETTE_INIT_MEMBER(clshroad_state,clshroad)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	for (i = 0;i < 256;i++)
		palette.set_pen_color(i,  pal4bit(color_prom[i + 256 * 0]),
										pal4bit(color_prom[i + 256 * 1]),
										pal4bit(color_prom[i + 256 * 2]));
}

PALETTE_INIT_MEMBER(clshroad_state,firebatl)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (i = 0; i < 0x200; i++)
		palette.set_pen_indirect(i, i & 0xff);

	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry = ((color_prom[(i - 0x200) + 0x000] & 0x0f) << 4) |
							(color_prom[(i - 0x200) + 0x100] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
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

TILE_GET_INFO_MEMBER(clshroad_state::get_tile_info_0a)
{
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code    =   m_vram_0[ tile_index * 2 + 0x40 ];
//  color   =   m_vram_0[ tile_index * 2 + 0x41 ];
	SET_TILE_INFO_MEMBER(1,
			code,
			0,
			0);
}

TILE_GET_INFO_MEMBER(clshroad_state::get_tile_info_0b)
{
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code    =   m_vram_0[ tile_index * 2 + 0x00 ];
//  color   =   m_vram_0[ tile_index * 2 + 0x01 ];
	SET_TILE_INFO_MEMBER(1,
			code,
			0,
			0);
}

WRITE8_MEMBER(clshroad_state::vram_0_w)
{
	int tile_index = offset / 2;
	int tile = (tile_index & 0x1f) + (tile_index & ~0x3f)/2;
	m_vram_0[offset] = data;
	if (tile_index & 0x20)  m_tilemap_0a->mark_tile_dirty(tile);
	else                    m_tilemap_0b->mark_tile_dirty(tile);
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
TILEMAP_MAPPER_MEMBER(clshroad_state::tilemap_scan_rows_extra)
{
	// The leftmost columns come from the bottom rows
	if (col <= 0x01)    return row + (col + 0x1e) * 0x20;
	// The rightmost columns come from the top rows
	if (col >= 0x22)    return row + (col - 0x22) * 0x20;

	// These are not visible, but *must* be mapped to other tiles than
	// those used by the leftmost and rightmost columns (tilemap "bug"?)
	if (row <= 0x01)    return 0;
	if (row >= 0x1e)    return 0;

	// "normal" layout for the rest.
	return (col-2) + row * 0x20;
}

TILE_GET_INFO_MEMBER(clshroad_state::get_tile_info_fb1)
{
	UINT8 code  =   m_vram_1[ tile_index + 0x000 ];
	UINT8 color =   m_vram_1[ tile_index + 0x400 ] & 0x3f;
	tileinfo.group = color;
	SET_TILE_INFO_MEMBER(2,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(clshroad_state::get_tile_info_1)
{
	UINT8 code  =   m_vram_1[ tile_index + 0x000 ];
	UINT8 color =   m_vram_1[ tile_index + 0x400 ];
	SET_TILE_INFO_MEMBER(2,
			code + ((color & 0xf0)<<4),
			color & 0x0f,
			0);
}

WRITE8_MEMBER(clshroad_state::vram_1_w)
{
	m_vram_1[offset] = data;
	m_tilemap_1->mark_tile_dirty(offset % 0x400);
}


VIDEO_START_MEMBER(clshroad_state,firebatl)
{
	/* These 2 use the graphics and scroll value */
	m_tilemap_0a = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_0a),this),TILEMAP_SCAN_ROWS,16,16,0x20,0x10);
	m_tilemap_0b = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_0b),this),TILEMAP_SCAN_ROWS,16,16,0x20,0x10);
	/* Text (No scrolling) */
	m_tilemap_1  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_fb1),this),tilemap_mapper_delegate(FUNC(clshroad_state::tilemap_scan_rows_extra),this),8,8,0x24,0x20);

	m_tilemap_0a->set_scroll_rows(1);
	m_tilemap_0b->set_scroll_rows(1);
	m_tilemap_1->set_scroll_rows(1);

	m_tilemap_0a->set_scroll_cols(1);
	m_tilemap_0b->set_scroll_cols(1);
	m_tilemap_1->set_scroll_cols(1);

	m_tilemap_0a->set_scrolldx(-0x30, -0xb5);
	m_tilemap_0b->set_scrolldx(-0x30, -0xb5);

	m_tilemap_0b->set_transparent_pen(0 );
	m_tilemap_1->configure_groups(*m_gfxdecode->gfx(2), 0x0f);
}

VIDEO_START_MEMBER(clshroad_state,clshroad)
{
	/* These 2 use the graphics and scroll value */
	m_tilemap_0a = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_0a),this),TILEMAP_SCAN_ROWS,16,16,0x20,0x10);
	m_tilemap_0b = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_0b),this),TILEMAP_SCAN_ROWS,16,16,0x20,0x10);
	/* Text (No scrolling) */
	m_tilemap_1  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(clshroad_state::get_tile_info_1),this),tilemap_mapper_delegate(FUNC(clshroad_state::tilemap_scan_rows_extra),this),8,8,0x24,0x20);

	m_tilemap_0a->set_scroll_rows(1);
	m_tilemap_0b->set_scroll_rows(1);
	m_tilemap_1->set_scroll_rows(1);

	m_tilemap_0a->set_scroll_cols(1);
	m_tilemap_0b->set_scroll_cols(1);
	m_tilemap_1->set_scroll_cols(1);

	m_tilemap_0a->set_scrolldx(-0x30, -0xb5);
	m_tilemap_0b->set_scrolldx(-0x30, -0xb5);

	m_tilemap_0b->set_transparent_pen(0x0f );
	m_tilemap_1->set_transparent_pen(0x0f );
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

void clshroad_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < m_spriteram.bytes() ; i += 8)
	{
		int y       =    240 - m_spriteram[i+1];
		int code    =   (m_spriteram[i+3] & 0x3f) + (m_spriteram[i+2] << 6);
		int x       =    m_spriteram[i+5]         + (m_spriteram[i+6] << 8);
		int attr    =    m_spriteram[i+7];

		int flipx   =   0;
		int flipy   =   0;

		x -= 0x4a/2;
		if (flip_screen())
		{
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				attr & 0x0f,
				flipx,flipy,
				x,y,15 );
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

UINT32 clshroad_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx  = m_vregs[ 0 ] + (m_vregs[ 1 ] << 8);
//  int priority = m_vregs[ 2 ];

	/* Only horizontal scrolling (these 2 layers use the same value) */
	m_tilemap_0a->set_scrollx(0, scrollx);
	m_tilemap_0b->set_scrollx(0, scrollx);

	m_tilemap_0a->draw(screen, bitmap, cliprect, 0,0);  // Opaque
	m_tilemap_0b->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
