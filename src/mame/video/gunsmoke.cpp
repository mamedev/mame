// license:BSD-3-Clause
// copyright-holders:Paul Leaman
#include "emu.h"
#include "includes/gunsmoke.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gunsmoke has three 256x4 palette PROMs (one per gun) and a lot ;-) of
  256x4 lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(gunsmoke_state, gunsmoke)
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

	/* characters use colors 0x40-0x4f */
	for (i = 0; i < 0x80; i++)
	{
		UINT8 ctabentry = color_prom[i] | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* background tiles use colors 0-0x3f */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x03) << 4);
		palette.set_pen_indirect(i - 0x80, ctabentry);
	}

	/* sprites use colors 0x80-0xff */
	for (i = 0x300; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x07) << 4) | 0x80;
		palette.set_pen_indirect(i - 0x180, ctabentry);
	}
}

WRITE8_MEMBER(gunsmoke_state::gunsmoke_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gunsmoke_state::gunsmoke_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gunsmoke_state::gunsmoke_c804_w)
{
	/* bits 0 and 1 are for coin counters */
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
	machine().bookkeeping().coin_counter_w(0, data & 0x02);

	/* bits 2 and 3 select the ROM bank */
	membank("bank1")->set_entry((data & 0x0c) >> 2);

	/* bit 5 resets the sound CPU? - we ignore it */

	/* bit 6 flips screen */
	flip_screen_set(data & 0x40);

	/* bit 7 enables characters? */
	m_chon = data & 0x80;
}

WRITE8_MEMBER(gunsmoke_state::gunsmoke_d806_w)
{
	/* bits 0-2 select the sprite 3 bank */
	m_sprite3bank = data & 0x07;

	/* bit 4 enables bg 1? */
	m_bgon = data & 0x10;

	/* bit 5 enables sprites? */
	m_objon = data & 0x20;
}

TILE_GET_INFO_MEMBER(gunsmoke_state::get_bg_tile_info)
{
	UINT8 *tilerom = memregion("gfx4")->base();

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs] + ((attr & 0x01) << 8);
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(gunsmoke_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xe0) << 2);
	int color = attr & 0x1f;

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void gunsmoke_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gunsmoke_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS,  32, 32, 2048, 8);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gunsmoke_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x4f);
}

void gunsmoke_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 32; offs >= 0; offs -= 32)
	{
		int attr = spriteram[offs + 1];
		int bank = (attr & 0xc0) >> 6;
		int code = spriteram[offs];
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x10;
		int sx = spriteram[offs + 3] - ((attr & 0x20) << 3);
		int sy = spriteram[offs + 2];

		if (bank == 3)
			bank += m_sprite3bank;

		code += 256 * bank;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}

UINT32 gunsmoke_state::screen_update_gunsmoke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx[0] + 256 * m_scrollx[1]);
	m_bg_tilemap->set_scrolly(0, m_scrolly[0]);

	if (m_bgon)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_objon)
		draw_sprites(bitmap, cliprect);

	if (m_chon)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
