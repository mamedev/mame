// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/cop01.h"


PALETTE_INIT_MEMBER(cop01_state, cop01)
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

	/* characters use colors 0x00-0x0f (or 0x00-0x7f, but the eight rows are identical) */
	for (i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	/* background tiles use colors 0xc0-0xff */
	/* I don't know how much of the lookup table PROM is hooked up, */
	/* I'm only using the first 32 bytes because the rest is empty. */
	for (i = 0x10; i < 0x90; i++)
	{
		UINT8 ctabentry = 0xc0 | ((i - 0x10) & 0x30) |
							(color_prom[(((i - 0x10) & 0x40) >> 2) | ((i - 0x10) & 0x0f)] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites use colors 0x80-0x8f (or 0x80-0xbf, but the four rows are identical) */
	for (i = 0x90; i < 0x190; i++)
	{
		UINT8 ctabentry = 0x80 | (color_prom[i - 0x90 + 0x100] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(cop01_state::get_bg_tile_info)
{
	int tile = m_bgvideoram[tile_index];
	int attr = m_bgvideoram[tile_index + 0x800];
	int pri = (attr & 0x80) >> 7;

	/* kludge: priority is not actually pen based, but color based. Since the
	 * game uses a lookup table, the two are not the same thing.
	 * Palette entries with bits 2&3 set have priority over sprites.
	 * tilemap.c can't handle that yet, so I'm cheating, because I know that
	 * color codes using the second row of the lookup table don't use palette
	 * entries 12-15.
	 * The only place where this has any effect is the beach at the bottom of
	 * the screen right at the beginning of mightguy. cop01 doesn't seem to
	 * use priority at all.
	 */
	if (attr & 0x10)
		pri = 0;

	SET_TILE_INFO_MEMBER(1, tile + ((attr & 0x03) << 8), (attr & 0x1c) >> 2, 0);
	tileinfo.group = pri;
}

TILE_GET_INFO_MEMBER(cop01_state::get_fg_tile_info)
{
	int tile = m_fgvideoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tile, 0, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void cop01_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cop01_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cop01_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(15);

	/* priority doesn't exactly work this way, see above */
	m_bg_tilemap->set_transmask(0, 0xffff, 0x0000); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x0fff, 0xf000); /* split type 1 has pens 0-11 transparent in front half */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(cop01_state::cop01_background_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(cop01_state::cop01_foreground_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(cop01_state::cop01_vreg_w)
{
	/*  0x40: --xx---- sprite bank, coin counters, flip screen
	 *        -----x-- flip screen
	 *        ------xx coin counters
	 *  0x41: xxxxxxxx xscroll
	 *  0x42: ---xx--- ? matches the bg tile color most of the time, but not
	 *                 during level transitions. Maybe sprite palette bank?
	 *                 (the four banks in the PROM are identical)
	 *        ------x- unused (xscroll overflow)
	 *        -------x msb xscroll
	 *  0x43: xxxxxxxx yscroll
	 */
	m_vreg[offset] = data;

	if (offset == 0)
	{
		machine().bookkeeping().coin_counter_w(0, data & 1);
		machine().bookkeeping().coin_counter_w(1, data & 2);
		flip_screen_set(data & 4);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void cop01_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs, code, attr, sx, sy, flipx, flipy, color;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		code = m_spriteram[offs + 1];
		attr = m_spriteram[offs + 2];
		/* xxxx---- color
		 * ----xx-- flipy,flipx
		 * -------x msbx
		 */
		color = attr>>4;
		flipx = attr & 0x04;
		flipy = attr & 0x08;

		sx = (m_spriteram[offs + 3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (code & 0x80)
			code += (m_vreg[0] & 0x30) << 3;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code,
			color,
			flipx,flipy,
			sx,sy,0 );
	}
}


UINT32 cop01_state::screen_update_cop01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_vreg[1] + 256 * (m_vreg[2] & 1));
	m_bg_tilemap->set_scrolly(0, m_vreg[3]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0 );
	return 0;
}
