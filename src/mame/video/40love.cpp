// license:???
// copyright-holders:Jarek Burczynski
/*
*   Video Driver for Forty-Love
*/

#include "emu.h"
#include "includes/40love.h"


/*
*   color prom decoding
*/

PALETTE_INIT_MEMBER(fortyl_state, fortyl)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[2*palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[2*palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[2*palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[2*palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r,g,b));

		color_prom++;
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
colorram format (2 bytes per one tilemap character line, 8 pixels height):

    offset 0    x... ....   x scroll (1 MSB bit)
    offset 0    .xxx x...   tile bank (see code below for banking formula)
    offset 0    .... .xxx   tiles color (one color code per whole tilemap line)

    offset 1    xxxx xxxx   x scroll (8 LSB bits)
*/

TILE_GET_INFO_MEMBER(fortyl_state::get_bg_tile_info)
{
	int tile_number = m_videoram[tile_index];
	int tile_attrib = m_colorram[(tile_index / 64) * 2];
	int tile_h_bank = (tile_attrib & 0x40) << 3;    /* 0x40->0x200 */
	int tile_l_bank = (tile_attrib & 0x18) << 3;    /* 0x10->0x80, 0x08->0x40 */

	int code = tile_number;
	if ((tile_attrib & 0x20) && (code >= 0xc0))
		code = (code & 0x3f) | tile_l_bank | 0x100;
	code |= tile_h_bank;

	SET_TILE_INFO_MEMBER(0,
			code,
			tile_attrib & 0x07,
			0);
}

/***************************************************************************

  State-related callbacks

***************************************************************************/

void fortyl_state::redraw_pixels()
{
	m_pix_redraw = 1;
	m_bg_tilemap->mark_all_dirty();
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void fortyl_state::video_start()
{
	m_pixram1 = auto_alloc_array_clear(machine(), UINT8, 0x4000);
	m_pixram2 = auto_alloc_array_clear(machine(), UINT8, 0x4000);

	m_tmp_bitmap1 = auto_bitmap_ind16_alloc(machine(), 256, 256);
	m_tmp_bitmap2 = auto_bitmap_ind16_alloc(machine(), 256, 256);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fortyl_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_xoffset = 128;    // this never changes

	m_bg_tilemap->set_scroll_rows(32);
	m_bg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_flipscreen));
	save_item(NAME(m_pix_color));
	save_pointer(NAME(m_pixram1), 0x4000);
	save_pointer(NAME(m_pixram2), 0x4000);
	save_item(NAME(*m_tmp_bitmap1));
	save_item(NAME(*m_tmp_bitmap2));
	save_item(NAME(m_pixram_sel));
	machine().save().register_postload(save_prepost_delegate(FUNC(fortyl_state::redraw_pixels), this));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void fortyl_state::fortyl_set_scroll_x( int offset )
{
	int i = offset & ~1;
	int x = ((m_colorram[i] & 0x80) << 1) | m_colorram[i + 1];    /* 9 bits signed */

	if (m_flipscreen)
		x += 0x51;
	else
		x -= 0x50;

	x &= 0x1ff;
	if (x & 0x100) x -= 0x200;              /* sign extend */

	m_bg_tilemap->set_scrollx(offset / 2, x);
}

WRITE8_MEMBER(fortyl_state::fortyl_pixram_sel_w)
{
	int offs;
	int f = data & 0x01;

	m_pixram_sel = (data & 0x04) >> 2;

	if (m_flipscreen != f)
	{
		m_flipscreen = f;
		flip_screen_set(m_flipscreen);
		m_pix_redraw = 1;

		for (offs = 0; offs < 32; offs++)
			fortyl_set_scroll_x(offs * 2);
	}
}

READ8_MEMBER(fortyl_state::fortyl_pixram_r)
{
	if (m_pixram_sel)
		return m_pixram2[offset];
	else
		return m_pixram1[offset];
}

void fortyl_state::fortyl_plot_pix( int offset )
{
	int x, y, i, c, d1, d2;

	x = (offset & 0x1f) * 8;
	y = (offset >> 5) & 0xff;

	if (m_pixram_sel)
	{
		d1 = m_pixram2[offset];
		d2 = m_pixram2[offset + 0x2000];
	}
	else
	{
		d1 = m_pixram1[offset];
		d2 = m_pixram1[offset + 0x2000];
	}

	for (i = 0; i < 8; i++)
	{
		c = ((d2 >> i) & 1) + ((d1 >> i) & 1) * 2;
		if (m_pixram_sel)
			m_tmp_bitmap2->pix16(y, x + i) = m_pix_color[c];
		else
			m_tmp_bitmap1->pix16(y, x + i) = m_pix_color[c];
	}
}

WRITE8_MEMBER(fortyl_state::fortyl_pixram_w)
{
	if (m_pixram_sel)
		m_pixram2[offset] = data;
	else
		m_pixram1[offset] = data;

	fortyl_plot_pix(offset & 0x1fff);
}


WRITE8_MEMBER(fortyl_state::fortyl_bg_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(fortyl_state::fortyl_bg_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(fortyl_state::fortyl_bg_colorram_w)
{
	if (m_colorram[offset] != data)
	{
		int i;

		m_colorram[offset] = data;
		for (i = (offset / 2) * 64; i < (offset / 2) * 64 + 64; i++)
			m_bg_tilemap->mark_tile_dirty(i);

		fortyl_set_scroll_x(offset);
	}
}

READ8_MEMBER(fortyl_state::fortyl_bg_colorram_r)
{
	return m_colorram[offset];
}

/***************************************************************************

  Display refresh

***************************************************************************/
/*
spriteram format (4 bytes per sprite):

    offset  0   xxxxxxxx    y position

    offset  1   x.......    flip Y
    offset  1   .x......    flip X
    offset  1   ..xxxxxx    gfx code (6 LSB bits)

    offset  2   ...xx...    gfx code (2 MSB bits)
    offset  2   .....xxx    color code
    offset  2   ???.....    ??? (not used, always 0)

    offset  3   xxxxxxxx    x position
*/

void fortyl_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
	int offs;

	/* spriteram #1 */
	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code, color, sx, sy, flipx, flipy;

		sx = spriteram[offs + 3];
		sy = spriteram[offs + 0] +1;

		if (m_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram[offs + 1] & 0x3f) + ((spriteram[offs + 2] & 0x18) << 3);
		flipx = ((spriteram[offs + 1] & 0x40) >> 6) ^ m_flipscreen;
		flipy = ((spriteram[offs + 1] & 0x80) >> 7) ^ m_flipscreen;
		color = (spriteram[offs + 2] & 0x07) + 0x08;

		if (spriteram[offs + 2] & 0xe0)
			color = machine().rand() & 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx+m_xoffset,sy,0);
	}

	/* spriteram #2 */
	for (offs = 0; offs < m_spriteram2.bytes(); offs += 4)
	{
		int code, color, sx, sy, flipx, flipy;

		sx = spriteram_2[offs + 3];
		sy = spriteram_2[offs + 0] +1;

		if (m_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram_2[offs + 1] & 0x3f) + ((spriteram_2[offs + 2] & 0x18) << 3);
		flipx = ((spriteram_2[offs + 1] & 0x40) >> 6) ^ m_flipscreen;
		flipy = ((spriteram_2[offs + 1] & 0x80) >> 7) ^ m_flipscreen;
		color = (spriteram_2[offs + 2] & 0x07) + 0x08;

		if (spriteram_2[offs + 2] & 0xe0)
			color = machine().rand() & 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx+m_xoffset,sy,0);
	}
}

void fortyl_state::draw_pixram( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	int f = m_flipscreen ^ 1;

	if (m_pix_redraw)
	{
		m_pix_redraw = 0;

		for (offs = 0; offs < 0x2000; offs++)
			fortyl_plot_pix(offs);
	}

	if (m_pixram_sel)
		copybitmap(bitmap, *m_tmp_bitmap1, f, f, m_xoffset, 0, cliprect);
	else
		copybitmap(bitmap, *m_tmp_bitmap2, f, f, m_xoffset, 0, cliprect);
}

UINT32 fortyl_state::screen_update_fortyl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_pixram(bitmap, cliprect);

	m_bg_tilemap->set_scrolldy(- m_video_ctrl[1] + 1, - m_video_ctrl[1] - 1 );
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}
