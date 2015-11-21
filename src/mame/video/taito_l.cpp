// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "includes/taito_l.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(taitol_state::get_bg18_tile_info)
{
	int attr = m_rambanks[2 * tile_index + 0x8000 + 1];
	int code = m_rambanks[2 * tile_index + 0x8000]
			| ((attr & 0x03) << 8)
			| ((m_bankc[(attr & 0xc) >> 2]) << 10)
			| (m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(taitol_state::get_bg19_tile_info)
{
	int attr = m_rambanks[2 * tile_index + 0x9000 + 1];
	int code = m_rambanks[2 * tile_index + 0x9000]
			| ((attr & 0x03) << 8)
			| ((m_bankc[(attr & 0xc) >> 2]) << 10)
			| (m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(taitol_state::get_ch1a_tile_info)
{
	int attr = m_rambanks[2 * tile_index + 0xa000 + 1];
	int code = m_rambanks[2 * tile_index + 0xa000] | ((attr & 0x01) << 8) | ((attr & 0x04) << 7);

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(taitol_state,taitol)
{
	int i;

	m_bg18_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_bg18_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg19_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_bg19_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_ch1a_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taitol_state::get_ch1a_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg18_tilemap->set_transparent_pen(0);
	m_ch1a_tilemap->set_transparent_pen(0);

	for (i = 0; i < 256; i++)
		m_palette->set_pen_color(i, rgb_t(0, 0, 0));

	m_ch1a_tilemap->set_scrolldx(-8, -8);
	m_bg18_tilemap->set_scrolldx(28, -11);
	m_bg19_tilemap->set_scrolldx(38, -21);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(taitol_state::horshoes_bankg_w)
{
	if (m_horshoes_gfxbank != data)
	{
		m_horshoes_gfxbank = data;

		m_bg18_tilemap->mark_all_dirty();
		m_bg19_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(taitol_state::taitol_bankc_w)
{
	if (m_bankc[offset] != data)
	{
		m_bankc[offset] = data;
//      logerror("Bankc %d, %02x (%04x)\n", offset, data, space.device().safe_pc());

		m_bg18_tilemap->mark_all_dirty();
		m_bg19_tilemap->mark_all_dirty();
	}
}

READ8_MEMBER(taitol_state::taitol_bankc_r)
{
	return m_bankc[offset];
}


WRITE8_MEMBER(taitol_state::taitol_control_w)
{
//  logerror("Control Write %02x (%04x)\n", data, space.device().safe_pc());

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
//  logerror("Control Read %02x (%04x)\n", cur_ctrl, space.device().safe_pc());
	return m_cur_ctrl;
}

void taitol_state::taitol_chardef14_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 0);
}

void taitol_state::taitol_chardef15_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 128);
}

void taitol_state::taitol_chardef16_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 256);
}

void taitol_state::taitol_chardef17_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 384);
}

void taitol_state::taitol_chardef1c_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 512);
}

void taitol_state::taitol_chardef1d_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 640);
}

void taitol_state::taitol_chardef1e_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 768);
}

void taitol_state::taitol_chardef1f_m( int offset )
{
	m_gfxdecode->gfx(2)->mark_dirty(offset / 32 + 896);
}

void taitol_state::taitol_bg18_m( int offset )
{
	m_bg18_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_state::taitol_bg19_m( int offset )
{
	m_bg19_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_state::taitol_char1a_m( int offset )
{
	m_ch1a_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_state::taitol_obj1b_m( int offset )
{
#if 0
	if (offset >= 0x3f0 && offset <= 0x3ff)
	{
		/* scroll, handled in vh-screenrefresh */
	}
#endif
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
	for (offs = 0; offs < TAITOL_SPRITERAM_SIZE - 3 * 8; offs += 8)
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


UINT32 taitol_state::screen_update_taitol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int dx, dy;

	dx = m_rambanks[0xb3f4] | (m_rambanks[0xb3f5] << 8);
	if (m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = m_rambanks[0xb3f6];

	m_bg18_tilemap->set_scrollx(0, -dx);
	m_bg18_tilemap->set_scrolly(0, -dy);

	dx = m_rambanks[0xb3fc] | (m_rambanks[0xb3fd] << 8);
	if (m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = m_rambanks[0xb3fe];

	m_bg19_tilemap->set_scrollx(0, -dx);
	m_bg19_tilemap->set_scrolly(0, -dy);

	if (m_cur_ctrl & 0x20)  /* display enable */
	{
		screen.priority().fill(0, cliprect);

		m_bg19_tilemap->draw(screen, bitmap, cliprect, 0, 0);

		if (m_cur_ctrl & 0x08)  /* sprites always over BG1 */
			m_bg18_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		else                    /* split priority */
			m_bg18_tilemap->draw(screen, bitmap, cliprect, 0,1);

		draw_sprites(screen, bitmap, cliprect);

		m_ch1a_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->pen(0), cliprect);
	return 0;
}



void taitol_state::screen_eof_taitol(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		UINT8 *spriteram = m_rambanks + 0xb000;

		memcpy(m_buff_spriteram, spriteram, TAITOL_SPRITERAM_SIZE);
	}
}
