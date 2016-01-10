// license:???
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/1942.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  1942 has three 256x4 palette PROMs (one per gun) and three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void _1942_state::create_palette()
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0 * 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 0 * 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 0 * 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 0 * 256] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 1 * 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 1 * 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 1 * 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 1 * 256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2 * 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 2 * 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 2 * 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 2 * 256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		m_palette->set_indirect_color(i,rgb_t(r,g,b));
	}
}

PALETTE_INIT_MEMBER(_1942_state,1942)
{
	create_palette();

	const UINT8 *color_prom = memregion("proms")->base();
	int i, colorbase;
	color_prom += 3 * 256;
	/* color_prom now points to the beginning of the lookup table */


	/* characters use palette entries 128-143 */
	colorbase = 0;
	for (i = 0; i < 64 * 4; i++)
	{
		m_palette->set_pen_indirect(colorbase + i, 0x80 | *color_prom++);
	}
	colorbase += 64 * 4;

	/* background tiles use palette entries 0-63 in four banks */
	for (i = 0; i < 32 * 8; i++)
	{
		m_palette->set_pen_indirect(colorbase + 0 * 32 * 8 + i, 0x00 | *color_prom);
		m_palette->set_pen_indirect(colorbase + 1 * 32 * 8 + i, 0x10 | *color_prom);
		m_palette->set_pen_indirect(colorbase + 2 * 32 * 8 + i, 0x20 | *color_prom);
		m_palette->set_pen_indirect(colorbase + 3 * 32 * 8 + i, 0x30 | *color_prom);
		color_prom++;
	}
	colorbase += 4 * 32 * 8;

	/* sprites use palette entries 64-79 */
	for (i = 0; i < 16 * 16; i++)
		m_palette->set_pen_indirect(colorbase + i, 0x40 | *color_prom++);
}

PALETTE_INIT_MEMBER(_1942_state,1942p)
{
	for (int i = 0; i < 0x400; i++)
	{
		palette.set_pen_indirect(i, i);
	}

	const UINT8 *color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		palette.set_pen_indirect(i+0x400, color_prom[i]| 0x240);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(_1942_state::get_fg_tile_info)
{
	int code, color;

	code = m_fg_videoram[tile_index];
	color = m_fg_videoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(0,
			code + ((color & 0x80) << 1),
			color & 0x3f,
			0);
}

TILE_GET_INFO_MEMBER(_1942_state::get_bg_tile_info)
{
	int code, color;

	tile_index = (tile_index & 0x0f) | ((tile_index & 0x01f0) << 1);

	code = m_bg_videoram[tile_index];
	color = m_bg_videoram[tile_index + 0x10];
	SET_TILE_INFO_MEMBER(1,
			code + ((color & 0x80) << 1),
			(color & 0x1f) + (0x20 * m_palette_bank),
			TILE_FLIPYX((color & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void _1942_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(_1942_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(_1942_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(0);
}

void _1942_state::video_start_c1942p()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(_1942_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(_1942_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(3);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(_1942_state::c1942_fgvideoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(_1942_state::c1942_bgvideoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty((offset & 0x0f) | ((offset >> 1) & 0x01f0));
}


WRITE8_MEMBER(_1942_state::c1942_palette_bank_w)
{
	if (m_palette_bank != data)
	{
		m_palette_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(_1942_state::c1942_scroll_w)
{
	m_scroll[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll[0] | (m_scroll[1] << 8));
}


WRITE8_MEMBER(_1942_state::c1942_c804_w)
{
	/* bit 7: flip screen
	   bit 4: cpu B reset
	   bit 0: coin counter */

	machine().bookkeeping().coin_counter_w(0,data & 0x01);

	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	flip_screen_set(data & 0x80);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void _1942_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int i, code, col, sx, sy, dir;

		code = (m_spriteram[offs] & 0x7f) + 4 * (m_spriteram[offs + 1] & 0x20)
				+ 2 * (m_spriteram[offs] & 0x80);
		col = m_spriteram[offs + 1] & 0x0f;
		sx = m_spriteram[offs + 3] - 0x10 * (m_spriteram[offs + 1] & 0x10);
		sy = m_spriteram[offs + 2];
		dir = 1;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			dir = -1;
		}

		/* handle double / quadruple height */
		i = (m_spriteram[offs + 1] & 0xc0) >> 6;
		if (i == 2)
			i = 3;

		do
		{
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code + i,col,
					flip_screen(),flip_screen(),
					sx,sy + 16 * i * dir,15);

			i--;
		} while (i >= 0);
	}


}

UINT32 _1942_state::screen_update_1942(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void _1942_state::draw_sprites_p( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int i, code, col, sx, sy, dir;

		code = (m_spriteram[offs] & 0x7f) + 4 * (m_spriteram[offs + 3] & 0x20)
				+ 2 * (m_spriteram[offs] & 0x80);
		col = m_spriteram[offs + 3] & 0x0f;


		sx = m_spriteram[offs + 2] - 0x10 * (m_spriteram[offs + 3] & 0x10);
		sy = m_spriteram[offs + 1];



		if (flip_screen())
		{
			sx = 240 - sx;
			dir = -1;
		}
		else
		{
			sy = 240 - sy;
			dir = 1;
		}

		/* handle double / quadruple height */
		i = (m_spriteram[offs + 3] & 0xc0) >> 6;
		if (i == 2)
			i = 3;

		i = 0;

		do
		{
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code + i,col,
					flip_screen(),flip_screen(),
					sx,sy + 16 * i * dir,15);

			i--;
		} while (i >= 0);
	}


}

UINT32 _1942_state::screen_update_1942p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites_p(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
