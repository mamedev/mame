// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  warpwarp.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/warpwarp.h"


static const rgb_t geebee_palette[] =
{
	rgb_t(0x00,0x00,0x00), /* black */
	rgb_t(0xff,0xff,0xff), /* white */
	rgb_t(0x7f,0x7f,0x7f)  /* grey  */
};

PALETTE_INIT_MEMBER(warpwarp_state,geebee)
{
	palette.set_pen_color(0, geebee_palette[0]);
	palette.set_pen_color(1, geebee_palette[1]);
	palette.set_pen_color(2, geebee_palette[1]);
	palette.set_pen_color(3, geebee_palette[0]);
	palette.set_pen_color(4, geebee_palette[0]);
	palette.set_pen_color(5, geebee_palette[2]);
	palette.set_pen_color(6, geebee_palette[2]);
	palette.set_pen_color(7, geebee_palette[0]);
}

PALETTE_INIT_MEMBER(warpwarp_state,navarone)
{
	palette.set_pen_color(0, geebee_palette[0]);
	palette.set_pen_color(1, geebee_palette[1]);
	palette.set_pen_color(2, geebee_palette[1]);
	palette.set_pen_color(3, geebee_palette[0]);
}


/***************************************************************************

  Warp Warp doesn't use PROMs - the 8-bit code is directly converted into a
  color.

  The color RAM is connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- BLUE
        -- 390 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- GREEN
        -- 1.6kohm resistor  -- GREEN
        -- 390 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
  bit 0 -- 1.6kohm resistor  -- RED

  Moreover, the bullet is pure white, obtained with three 220 ohm resistors.

***************************************************************************/

PALETTE_INIT_MEMBER(warpwarp_state,warpwarp)
{
	static const int resistances_tiles_rg[] = { 1600, 820, 390 };
	static const int resistances_tiles_b[]  = { 820, 390 };
	static const int resistance_ball[]      = { 220 };

	double weights_tiles_rg[3], weights_tiles_b[2], weight_ball[1];

	compute_resistor_weights(0, 0xff, -1.0,
								3, resistances_tiles_rg, weights_tiles_rg, 150, 0,
								2, resistances_tiles_b,  weights_tiles_b,  150, 0,
								1, resistance_ball,      weight_ball,      150, 0);

	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;
		int r,g,b;

		/* red component */
		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		bit2 = (i >> 2) & 0x01;
		r = combine_3_weights(weights_tiles_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 0x01;
		bit1 = (i >> 4) & 0x01;
		bit2 = (i >> 5) & 0x01;
		g = combine_3_weights(weights_tiles_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (i >> 6) & 0x01;
		bit1 = (i >> 7) & 0x01;
		b = combine_2_weights(weights_tiles_b, bit0, bit1);

		palette.set_pen_color((i * 2) + 0, rgb_t::black);
		palette.set_pen_color((i * 2) + 1, rgb_t(r, g, b));
	}

	palette.set_pen_color(0x200, rgb_t(weight_ball[0], weight_ball[0], weight_ball[0]));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 34x28 */
TILEMAP_MAPPER_MEMBER(warpwarp_state::tilemap_scan)
{
	int offs;

	row += 2;
	col--;
	if (col & 0x20)
		offs = row + ((col & 1) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

TILE_GET_INFO_MEMBER(warpwarp_state::geebee_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = (m_geebee_bgw & 1) | ((code & 0x80) >> 6);
	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::navarone_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = m_geebee_bgw & 1;
	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::warpwarp_get_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index],
			m_videoram[tile_index + 0x400],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(warpwarp_state,geebee)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(warpwarp_state::geebee_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}

VIDEO_START_MEMBER(warpwarp_state,navarone)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(warpwarp_state::navarone_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}

VIDEO_START_MEMBER(warpwarp_state,warpwarp)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(warpwarp_state::warpwarp_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(warpwarp_state::geebee_videoram_w)
{
	m_geebee_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(warpwarp_state::warpwarp_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

inline void warpwarp_state::plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen)
{
	if (cliprect.contains(x, y))
		bitmap.pix16(y, x) = pen;
}

void warpwarp_state::draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen)
{
	if (m_ball_on)
	{
		int x,y,i,j;

		if (flip_screen() & 1) {
			x = 376 - m_ball_h;
			y = 280 - m_ball_v;
		}
		else {
			x = 264 - m_ball_h;
			y = 240 - m_ball_v;
		}

		for (i = m_ball_sizey;i > 0;i--)
			for (j = m_ball_sizex;j > 0;j--)
				plot(bitmap, cliprect, x-j, y-i, pen);
	}
}

UINT32 warpwarp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	draw_ball(bitmap, cliprect, m_ball_pen);
	return 0;
}
