// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/dogfgt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Dog-Fight has both palette RAM and PROMs. The PROMs are used for tiles &
  pixmap, RAM for sprites.

***************************************************************************/

PALETTE_INIT_MEMBER(dogfgt_state, dogfgt)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* first 16 colors are RAM */
	for (i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i + 16, rgb_t(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(dogfgt_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_bgvideoram[tile_index],
			m_bgvideoram[tile_index + 0x400] & 0x03,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void dogfgt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dogfgt_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bitmapram = auto_alloc_array(machine(), UINT8, BITMAPRAM_SIZE);
	save_pointer(NAME(m_bitmapram), BITMAPRAM_SIZE);

	m_screen->register_screen_bitmap(m_pixbitmap);
	save_item(NAME(m_pixbitmap));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(dogfgt_state::dogfgt_plane_select_w)
{
	m_bm_plane = data;
}

READ8_MEMBER(dogfgt_state::dogfgt_bitmapram_r)
{
	if (m_bm_plane > 2)
	{
		popmessage("bitmapram_r offs %04x plane %d\n", offset, m_bm_plane);
		return 0;
	}

	return m_bitmapram[offset + BITMAPRAM_SIZE / 3 * m_bm_plane];
}

WRITE8_MEMBER(dogfgt_state::internal_bitmapram_w)
{
	int x, y, subx;

	m_bitmapram[offset] = data;

	offset &= (BITMAPRAM_SIZE / 3 - 1);
	x = 8 * (offset / 256);
	y = offset % 256;

	for (subx = 0; subx < 8; subx++)
	{
		int i, color = 0;

		for (i = 0; i < 3; i++)
			color |= ((m_bitmapram[offset + BITMAPRAM_SIZE / 3 * i] >> subx) & 1) << i;

		if (flip_screen())
			m_pixbitmap.pix16(y ^ 0xff, (x + subx) ^ 0xff) = PIXMAP_COLOR_BASE + 8 * m_pixcolor + color;
		else
			m_pixbitmap.pix16(y, x + subx) = PIXMAP_COLOR_BASE + 8 * m_pixcolor + color;
	}
}

WRITE8_MEMBER(dogfgt_state::dogfgt_bitmapram_w)
{
	if (m_bm_plane > 2)
	{
		popmessage("bitmapram_w offs %04x plane %d\n", offset, m_bm_plane);
		return;
	}

	internal_bitmapram_w(space, offset + BITMAPRAM_SIZE / 3 * m_bm_plane, data);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_scroll_w)
{
	m_scroll[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll[0] + 256 * m_scroll[1] + 256);
	m_bg_tilemap->set_scrolly(0, m_scroll[2] + 256 * m_scroll[3]);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_1800_w)
{
	/* bits 0 and 1 are probably text color (not verified because PROM is missing) */
	m_pixcolor = ((data & 0x01) << 1) | ((data & 0x02) >> 1);

	/* bits 4 and 5 are coin counters */
	coin_counter_w(machine(), 0, data & 0x10);
	coin_counter_w(machine(), 1, data & 0x20);

	/* bit 7 is screen flip */
	flip_screen_set(data & 0x80);

	/* other bits unused? */
	logerror("PC %04x: 1800 = %02x\n", space.device().safe_pc(), data);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void dogfgt_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] & 0x01)
		{
			int sx, sy, flipx, flipy;

			sx = m_spriteram[offs + 3];
			sy = (240 - m_spriteram[offs + 2]) & 0xff;
			flipx = m_spriteram[offs] & 0x04;
			flipy = m_spriteram[offs] & 0x02;
			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_spriteram[offs + 1] + ((m_spriteram[offs] & 0x30) << 4),
					(m_spriteram[offs] & 0x08) >> 3,
					flipx,flipy,
					sx,sy,0);
		}
	}
}


UINT32 dogfgt_state::screen_update_dogfgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	if (m_lastflip != flip_screen() || m_lastpixcolor != m_pixcolor)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		m_lastflip = flip_screen();
		m_lastpixcolor = m_pixcolor;

		for (offs = 0; offs < BITMAPRAM_SIZE; offs++)
			internal_bitmapram_w(space, offs, m_bitmapram[offs]);
	}


	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	copybitmap_trans(bitmap, m_pixbitmap, 0, 0, 0, 0, cliprect, PIXMAP_COLOR_BASE + 8 * m_pixcolor);
	return 0;
}
