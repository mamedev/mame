// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  warpwarp.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "warpwarp.h"


static constexpr rgb_t geebee_pens[] =
{
	rgb_t(0x00,0x00,0x00), /* black */
	rgb_t(0xff,0xff,0xff), /* white */
	rgb_t(0x7f,0x7f,0x7f)  /* grey  */
};

void warpwarp_state::geebee_palette(palette_device &palette) const
{
	palette.set_pen_color(0, geebee_pens[0]);
	palette.set_pen_color(1, geebee_pens[1]);
	palette.set_pen_color(2, geebee_pens[1]);
	palette.set_pen_color(3, geebee_pens[0]);
	palette.set_pen_color(4, geebee_pens[0]);
	palette.set_pen_color(5, geebee_pens[2]);
	palette.set_pen_color(6, geebee_pens[2]);
	palette.set_pen_color(7, geebee_pens[0]);
}

void warpwarp_state::navarone_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t::white());
	palette.set_pen_color(2, rgb_t::white());
	palette.set_pen_color(3, rgb_t::black());
}

void warpwarp_state::sos_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::white());
	palette.set_pen_color(1, rgb_t::black());
	palette.set_pen_color(2, rgb_t::black());
	palette.set_pen_color(3, rgb_t::white());
}

MACHINE_RESET_MEMBER(warpwarp_state,kaitei)
{
	// Some PCB videos/images shows a b&w arrangement, others a full colorized one.
	// This is due of the monitor type used, cfr. http://news.livedoor.com/article/detail/5604337/

	// We change color palette at reset time, according to the configuration switch.
	if (m_in_config->read() & 1) // color
	{
		m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00));
		m_palette->set_pen_color(1, rgb_t(0x00,0xff,0xff));
		m_palette->set_pen_color(2, rgb_t(0x00,0xff,0xff));
		m_palette->set_pen_color(3, rgb_t(0x00,0x00,0x00));
		m_palette->set_pen_color(4, rgb_t(0x00,0x00,0x00));
		m_palette->set_pen_color(5, rgb_t(0x00,0xff,0x00));
		m_palette->set_pen_color(6, rgb_t(0x00,0xff,0x00));
		m_palette->set_pen_color(7, rgb_t(0x00,0x00,0x00));
		m_palette->set_pen_color(8, rgb_t(0xff,0x00,0x00)); // ball pen
	}
	else // b & w
	{
		m_palette->set_pen_color(0, geebee_pens[0]);
		m_palette->set_pen_color(1, geebee_pens[1]);
		m_palette->set_pen_color(2, geebee_pens[1]);
		m_palette->set_pen_color(3, geebee_pens[0]);
		m_palette->set_pen_color(4, geebee_pens[0]);
		m_palette->set_pen_color(5, geebee_pens[1]);
		m_palette->set_pen_color(6, geebee_pens[1]);
		m_palette->set_pen_color(7, geebee_pens[0]);
		m_palette->set_pen_color(8, geebee_pens[1]); // ball pen
	}
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

void warpwarp_state::warpwarp_palette(palette_device &palette) const
{
	static constexpr int resistances_tiles_rg[] = { 1600, 820, 390 };
	static constexpr int resistances_tiles_b[]  = { 820, 390 };
	static constexpr int resistance_ball[]      = { 220 };

	double weights_tiles_rg[3], weights_tiles_b[2], weight_ball[1];
	compute_resistor_weights(0, 0xff, -1.0,
								3, resistances_tiles_rg, weights_tiles_rg, 150, 0,
								2, resistances_tiles_b,  weights_tiles_b,  150, 0,
								1, resistance_ball,      weight_ball,      150, 0);

	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 1);
		bit2 = BIT(i, 2);
		int const r = combine_weights(weights_tiles_rg, bit0, bit1, bit2);

		// green component
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		bit2 = BIT(i, 5);
		int const g = combine_weights(weights_tiles_rg, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(i, 6);
		bit1 = BIT(i, 7);
		int const b = combine_weights(weights_tiles_b, bit0, bit1);

		palette.set_pen_color((i << 1) | 0, rgb_t::black());
		palette.set_pen_color((i << 1) | 1, rgb_t(r, g, b));
	}

	palette.set_pen_color(0x200, rgb_t(weight_ball[0], weight_ball[0], weight_ball[0]));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 34x28 */
TILEMAP_MAPPER_MEMBER(warpwarp_state::tilemap_scan)
{
	row += 2;
	col--;
	if (col & 0x20)
		return row + ((col & 1) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(warpwarp_state::geebee_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = (m_geebee_bgw & 1) | ((code & 0x80) >> 6);
	tileinfo.set(0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::navarone_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = m_geebee_bgw & 1;
	tileinfo.set(0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::warpwarp_get_tile_info)
{
	tileinfo.set(0,
			m_videoram[tile_index],
			m_videoram[tile_index + 0x400],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(warpwarp_state,geebee)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(warpwarp_state::geebee_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(warpwarp_state::tilemap_scan)), 8,8, 34,28);
}

VIDEO_START_MEMBER(warpwarp_state,navarone)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(warpwarp_state::navarone_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(warpwarp_state::tilemap_scan)), 8,8, 34,28);
}

VIDEO_START_MEMBER(warpwarp_state,warpwarp)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(warpwarp_state::warpwarp_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(warpwarp_state::tilemap_scan)), 8,8, 34,28);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void warpwarp_state::geebee_videoram_w(offs_t offset, uint8_t data)
{
	m_geebee_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void warpwarp_state::warpwarp_videoram_w(offs_t offset, uint8_t data)
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
		bitmap.pix(y, x) = pen;
}

void warpwarp_state::draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen)
{
	if (m_ball_on)
	{
		int x,y,i,j;

		x = 264 - m_ball_h;
		y = 240 - m_ball_v;

		for (i = m_ball_sizey;i > 0;i--)
			for (j = m_ball_sizex;j > 0;j--)
				plot(bitmap, cliprect, x-j, y-i, pen);
	}
}

uint32_t warpwarp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	draw_ball(bitmap, cliprect, m_ball_pen);
	return 0;
}
