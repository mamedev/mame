// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

1943 Video Hardware

This board handles tile/tile and tile/sprite priority with a PROM. Its
working is hardcoded in the driver.

The PROM have address inputs wired as follows:

A0 bg (SCR) opaque
A1 bit 2 of sprite (OBJ) attribute (guess)
A2 bit 3 of sprite (OBJ) attribute (guess)
A3 sprite (OBJ) opaque
A4 fg (CHAR) opaque
A5 wired to mass
A6 wired to mass
A7 wired to mass

2 bits of the output selects the active layer, it can be:
(output & 0x03)
0 bg2 (SCR2)
1 bg (SCR)
2 sprite (OBJ)
3 fg (CHAR)

other 2 bits (output & 0x0c) unknown

***************************************************************************/

#include "emu.h"
#include "1943.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  1943 has three 256x4 palette PROMs (one per gun) and a lot ;-) of 256x4
  lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void _1943_state::_1943_palette(palette_device &palette) const
{
	const u8 *color_prom = m_proms;
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		const int r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		const int g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		const int b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0x40-0x4f
	for (int i = 0x00; i < 0x80; i++)
	{
		const u8 ctabentry = (color_prom[i] & 0x0f) | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	// foreground tiles use colors 0x00-0x3f
	for (int i = 0x80; i < 0x180; i++)
	{
		const u8 ctabentry =
				((color_prom[0x200 + (i - 0x080)] & 0x03) << 4) |
				((color_prom[0x100 + (i - 0x080)] & 0x0f) << 0);
		palette.set_pen_indirect(i, ctabentry);
	}

	// background tiles also use colors 0x00-0x3f
	for (int i = 0x180; i < 0x280; i++)
	{
		const u8 ctabentry =
				((color_prom[0x400 + (i - 0x180)] & 0x03) << 4) |
				((color_prom[0x300 + (i - 0x180)] & 0x0f) << 0);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites use colors 0x80-0xff
	   bit 3 of BMPROM.07 selects priority over the background,
	   but we handle it differently for speed reasons */
	for (int i = 0x280; i < 0x380; i++)
	{
		const u8 ctabentry =
				((color_prom[0x600 + (i - 0x280)] & 0x07) << 4) |
				((color_prom[0x500 + (i - 0x280)] & 0x0f) << 0) |
				0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void _1943_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void _1943_state::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void _1943_state::control_w(u8 data)
{
	/* bits 0 and 1 are coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bits 2, 3 and 4 select the ROM bank */
	m_mainbank->set_entry((data & 0x1c) >> 2);

	/* bit 5 resets the sound CPU */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 6 flips screen */
	flip_screen_set(data & 0x40);

	/* bit 7 enables characters */
	m_char_on = data & 0x80;
}

void _1943_state::layer_w(u8 data)
{
	/* bit 4 enables bg 1 */
	m_bg1_on = data & 0x10;

	/* bit 5 enables bg 2 */
	m_bg2_on = data & 0x20;

	/* bit 6 enables sprites */
	m_obj_on = data & 0x40;
}

TILE_GET_INFO_MEMBER(_1943_state::get_bg2_tile_info)
{
	const int offs = 0x8000 + (tile_index * 2);
	const u8 attr = m_tilerom[offs + 1];
	const u32 code = m_tilerom[offs];
	const u32 color = (attr & 0x3c) >> 2;
	const int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	tileinfo.set(2, code, color, flags);
}

TILE_GET_INFO_MEMBER(_1943_state::get_bg_tile_info)
{
	const int offs = tile_index * 2;
	const u8 attr = m_tilerom[offs + 1];
	const u32 code = m_tilerom[offs] + ((attr & 0x01) << 8);
	const u32 color = (attr & 0x3c) >> 2;
	const int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	tileinfo.group = color;
	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(_1943_state::get_fg_tile_info)
{
	const u8 attr = m_colorram[tile_index];
	const u32 code = m_videoram[tile_index] + ((attr & 0xe0) << 3);
	const u32 color = attr & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void _1943_state::video_start()
{
	m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1943_state::get_bg2_tile_info)), TILEMAP_SCAN_COLS, 32, 32, 2048, 8);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1943_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 32, 32, 2048, 8);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1943_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(1), 0x0f);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(  6,   6);
	m_bg2_tilemap->set_scrolldx(128, 128);
	m_bg2_tilemap->set_scrolldy(  6,   6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(  6,   6);

	save_item(NAME(m_char_on));
	save_item(NAME(m_obj_on));
	save_item(NAME(m_bg1_on));
	save_item(NAME(m_bg2_on));
}

void _1943_state::_1943_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color)
{
	bitmap_ind8 &priority_bitmap = m_screen->priority();
	/* Start drawing */
	const u16 pal = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const u8 *source_base = gfx->get_data(code % gfx->elements());

	const int xinc = flipx ? -1 : 1;
	const int yinc = flipy ? -1 : 1;

	int x_index_base = flipx ? gfx->width() - 1 : 0;
	int y_index = flipy ? gfx->height() - 1 : 0;

	// start coordinates
	int sx = offsx;
	int sy = offsy;

	// end coordinates
	int ex = sx + gfx->width();
	int ey = sy + gfx->height();

	if (sx < clip.min_x)
	{ // clip left
		const int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += xinc * pixels;
	}
	if (sy < clip.min_y)
	{ // clip top
		const int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += yinc * pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > clip.max_x + 1)
	{ // clip right
		ex = clip.max_x + 1;
	}
	if (ey > clip.max_y + 1)
	{ // clip bottom
		ey = clip.max_y + 1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything
		for (int y = sy; y < ey; y++)
		{
			u8 const *const source = source_base + y_index * gfx->rowbytes();
			u16 *const dest = &dest_bmp.pix(y);
			u8 *const pri = &priority_bitmap.pix(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				if (!(pri[x] & 0x80))
				{
					u8 const c = source[x_index];
					if (c != transparent_color)
					{
						// the priority is actually selected by bit 3 of BMPROM.07
						if (((pri[x] & 2) == 0) || ((m_proms[0x900 + c + (color << 4)] & 0x08) == 0))
							dest[x] = pal + c;

						pri[x] = 0xff; // mark it 'already drawn'
					}
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

void _1943_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 32)
	{
		const u8 attr = m_spriteram[offs + 1];
		const u32 code = m_spriteram[offs] + ((attr & 0xe0) << 3);
		const u32 color = attr & 0x0f;
		int sx = m_spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = m_spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		_1943_drawgfx(bitmap,cliprect, m_gfxdecode->gfx(3), code, color, flip_screen(), flip_screen(), sx+128, sy+6, 0);
	}
}

u32 _1943_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg2_tilemap->set_scrollx(0, m_bgscrollx[0] + 256 * m_bgscrollx[1]);
	m_bg_tilemap->set_scrollx(0, m_scrollx[0] + 256 * m_scrollx[1]);
	m_bg_tilemap->set_scrolly(0, m_scrolly[0]);

	screen.priority().fill(0, cliprect);

	if (m_bg2_on)
		m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_bg1_on)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	if (m_obj_on)
		draw_sprites(bitmap, cliprect);

	if (m_char_on)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
