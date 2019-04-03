// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "includes/taito_l.h"
#include "screen.h"

#include <algorithm>

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template<int Offset>
TILE_GET_INFO_MEMBER(taitol_state::get_tile_info)
{
	int attr = m_vram[2 * tile_index + Offset + 1];
	int code = m_vram[2 * tile_index + Offset]
			| ((attr & 0x03) << 8)
			| ((m_bankc[(attr & 0xc) >> 2]) << 10)
			| (m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(taitol_state::get_tx_tile_info)
{
	int attr = m_vram[2 * tile_index + 0xa000 + 1];
	int code = m_vram[2 * tile_index + 0xa000] | ((attr & 0x07) << 8);

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void taitol_state::video_start()
{
	m_buff_spriteram = make_unique_clear<u8[]>(SPRITERAM_SIZE);

	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_tile_info<0x8000>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_tile_info<0x9000>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_tx_tile_info),this),      TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx(-8, -8);
	m_bg_tilemap[0]->set_scrolldx(28, -11);
	m_bg_tilemap[1]->set_scrolldx(38, -21);

	save_pointer(NAME(m_buff_spriteram), SPRITERAM_SIZE);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(horshoes_state::bankg_w)
{
	if (m_horshoes_gfxbank != data)
	{
		m_horshoes_gfxbank = data;

		for (int i = 0; i < 2; i++)
			m_bg_tilemap[i]->mark_all_dirty();
	}
}

WRITE8_MEMBER(taitol_state::taitol_bankc_w)
{
	if (m_bankc[offset] != data)
	{
		m_bankc[offset] = data;
//      logerror("Bankc %d, %02x (%s)\n", offset, data, m_maincpu->pc());

		for (int i = 0; i < 2; i++)
			m_bg_tilemap[i]->mark_all_dirty();
	}
}

READ8_MEMBER(taitol_state::taitol_bankc_r)
{
	return m_bankc[offset];
}


WRITE8_MEMBER(taitol_state::taitol_control_w)
{
//  logerror("Control Write %02x (%s)\n", data, m_maincpu->pc());

	m_cur_ctrl = data;
//popmessage("%02x",data);

	/* bit 0 unknown */

	/* bit 1 unknown */

	/* bit 3 controls sprite/tile priority - handled in vh_screenrefresh() */

	/* bit 4 flip screen */
	m_flipscreen = data & 0x10;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* bit 5 display enable - handled in vh_screenrefresh() */
}

READ8_MEMBER(taitol_state::taitol_control_r)
{
//  logerror("Control Read %02x (%s)\n", cur_ctrl, m_maincpu->pc());
	return m_cur_ctrl;
}

WRITE8_MEMBER(taitol_state::vram_w)
{
	// TODO : 0x10000-0x13fff Unused?
	if ((offset & 0xc000) == 0)
		return;

	if((m_vram[offset] & mem_mask) == (data & mem_mask))
		return;

	COMBINE_DATA(&m_vram[offset]);
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32);
	switch (offset & 0xf000)
	{
		case 0x8000:
		case 0x9000:
			m_bg_tilemap[(offset >> 12) & 1]->mark_tile_dirty((offset & 0xfff) / 2);
			break;
		case 0xa000:
			m_tx_tilemap->mark_tile_dirty((offset & 0xfff) / 2);
			break;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

/*
    Sprite format:
    00: xxxxxxxx tile number (low)
    01: xxxxxxxx tile number (high)
    02: ----xxxx color
        ----x--- priority
    03: -------x flip x
        ------x- flip y
    04: xxxxxxxx x position (low)
    05: -------x x position (high)
    06: xxxxxxxx y position
    07: xxxxxxxx unknown / ignored? Seems just garbage in many cases, e.g
                 plgirs2 bullets and raimais big bosses.
*/

void taitol_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	/* at spriteram + 0x3f0 and 03f8 are the tilemap control registers; spriteram + 0x3e8 seems to be unused */
	for (offs = 0; offs < SPRITERAM_SIZE - 3 * 8; offs += 8)
	{
		int code, color, sx, sy, flipx, flipy;

		color = m_buff_spriteram[offs + 2] & 0x0f;
		code = m_buff_spriteram[offs] | (m_buff_spriteram[offs + 1] << 8);

		code |= (m_horshoes_gfxbank & 0x03) << 10;

		sx = m_buff_spriteram[offs + 4] | ((m_buff_spriteram[offs + 5] & 1) << 8);
		sy = m_buff_spriteram[offs + 6];
		if (sx >= 320)
			sx -= 512;
		flipx = m_buff_spriteram[offs + 3] & 0x01;
		flipy = m_buff_spriteram[offs + 3] & 0x02;

		if (m_flipscreen)
		{
			sx = 304 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				screen.priority(),
				(color & 0x08) ? 0xaa : 0x00,0);
	}
}


uint32_t taitol_state::screen_update_taitol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int dx, dy;

	dx = m_vram[0xb3f4] | (m_vram[0xb3f5] << 8);
	if (m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = m_vram[0xb3f6];

	m_bg_tilemap[0]->set_scrollx(0, -dx);
	m_bg_tilemap[0]->set_scrolly(0, -dy);

	dx = m_vram[0xb3fc] | (m_vram[0xb3fd] << 8);
	if (m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = m_vram[0xb3fe];

	m_bg_tilemap[1]->set_scrollx(0, -dx);
	m_bg_tilemap[1]->set_scrolly(0, -dy);

	if (m_cur_ctrl & 0x20)  /* display enable */
	{
		screen.priority().fill(0, cliprect);

		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

		if (m_cur_ctrl & 0x08)  /* sprites always over BG1 */
			m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		else                    /* split priority */
			m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,1);

		draw_sprites(screen, bitmap, cliprect);

		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->pen(0), cliprect);
	return 0;
}



WRITE_LINE_MEMBER(taitol_state::screen_vblank_taitol)
{
	// rising edge
	if (state)
	{
		std::copy(&m_vram[0xb000], &m_vram[0xb000+SPRITERAM_SIZE], &m_buff_spriteram[0]);
	}
}
