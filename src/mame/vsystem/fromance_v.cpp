// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi. Bryan McPhail, Nicola Salmoria, Aaron Giles
/******************************************************************************

    Video Hardware for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

******************************************************************************/

#include "emu.h"
#include "includes/fromance.h"




/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

inline void fromance_state::get_fromance_tile_info( tile_data &tileinfo, int tile_index, int layer )
{
	int tile = ((m_local_videoram[layer][0x0000 + tile_index] & 0x80) << 9) |
				(m_local_videoram[layer][0x1000 + tile_index] << 8) |
				m_local_videoram[layer][0x2000 + tile_index];
	int color = m_local_videoram[layer][tile_index] & 0x7f;

	tileinfo.set(layer, tile, color, 0);
}

TILE_GET_INFO_MEMBER(fromance_state::get_fromance_bg_tile_info){ get_fromance_tile_info(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(fromance_state::get_fromance_fg_tile_info){ get_fromance_tile_info(tileinfo, tile_index, 1); }


inline void fromance_state::get_nekkyoku_tile_info( tile_data &tileinfo, int tile_index, int layer )
{
	int tile = (m_local_videoram[layer][0x0000 + tile_index] << 8) |
				m_local_videoram[layer][0x1000 + tile_index];
	int color = m_local_videoram[layer][tile_index + 0x2000] & 0x3f;

	tileinfo.set(layer, tile, color, 0);
}

TILE_GET_INFO_MEMBER(fromance_state::get_nekkyoku_bg_tile_info){ get_nekkyoku_tile_info(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(fromance_state::get_nekkyoku_fg_tile_info){ get_nekkyoku_tile_info(tileinfo, tile_index, 1); }



/*************************************
 *
 *  Video system start
 *
 *************************************/

void fromance_state::init_common(  )
{
	/* allocate local videoram */
	m_local_videoram[0] = std::make_unique<uint8_t[]>(0x1000 * 3);
	m_local_videoram[1] = std::make_unique<uint8_t[]>(0x1000 * 3);

	/* allocate local palette RAM */
	m_local_paletteram = std::make_unique<uint8_t[]>(0x800 * 2);

	/* configure tilemaps */
	m_fg_tilemap->set_transparent_pen(15);

	/* reset the timer */
	m_crtc_timer = timer_alloc(FUNC(fromance_state::crtc_interrupt_gen), this);

	/* state save */
	save_item(NAME(m_selected_videoram));
	save_pointer(NAME(m_local_videoram[0]), 0x1000 * 3);
	save_pointer(NAME(m_local_videoram[1]), 0x1000 * 3);
	save_item(NAME(m_selected_paletteram));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_gfxreg));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_flipscreen_old));
	save_item(NAME(m_scrollx_ofs));
	save_item(NAME(m_scrolly_ofs));
	save_pointer(NAME(m_local_paletteram), 0x800 * 2);
}

VIDEO_START_MEMBER(fromance_state,fromance)
{
	/* allocate tilemaps */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fromance_state::get_fromance_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fromance_state::get_fromance_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);

	init_common();
}

VIDEO_START_MEMBER(fromance_state,nekkyoku)
{
	/* allocate tilemaps */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fromance_state::get_nekkyoku_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fromance_state::get_nekkyoku_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);

	init_common();
}

/*************************************
 *
 *  Graphics control register
 *
 *************************************/

void fromance_state::fromance_gfxreg_w(uint8_t data)
{
	m_gfxreg = data;
	m_flipscreen = (data & 0x01);
	m_selected_videoram = (~data >> 1) & 1;
	m_selected_paletteram = (data >> 6) & 1;

	if (m_flipscreen != m_flipscreen_old)
	{
		m_flipscreen_old = m_flipscreen;
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  Banked palette RAM
 *
 *************************************/

uint8_t fromance_state::fromance_paletteram_r(offs_t offset)
{
	/* adjust for banking and read */
	offset |= m_selected_paletteram << 11;
	return m_local_paletteram[offset];
}


void fromance_state::fromance_paletteram_w(offs_t offset, uint8_t data)
{
	int palword;

	/* adjust for banking and modify */
	offset |= m_selected_paletteram << 11;
	m_local_paletteram[offset] = data;

	/* compute R,G,B */
	palword = (m_local_paletteram[offset | 1] << 8) | m_local_paletteram[offset & ~1];
	m_palette->set_pen_color(offset / 2, pal5bit(palword >> 10), pal5bit(palword >> 5), pal5bit(palword >> 0));
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

uint8_t fromance_state::fromance_videoram_r(offs_t offset)
{
	return m_local_videoram[m_selected_videoram][offset];
}


void fromance_state::fromance_videoram_w(offs_t offset, uint8_t data)
{
	m_local_videoram[m_selected_videoram][offset] = data;
	(m_selected_videoram ? m_fg_tilemap : m_bg_tilemap)->mark_tile_dirty(offset & 0x0fff);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

void fromance_state::fromance_scroll_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
	{
		switch (offset)
		{
			case 0:
				m_scrollx[1] = (data + (((m_gfxreg & 0x08) >> 3) * 0x100) - m_scrollx_ofs);
				break;
			case 1:
				m_scrolly[1] = (data + (((m_gfxreg & 0x04) >> 2) * 0x100) - m_scrolly_ofs); // - 0x10
				break;
			case 2:
				m_scrollx[0] = (data + (((m_gfxreg & 0x20) >> 5) * 0x100) - m_scrollx_ofs);
				break;
			case 3:
				m_scrolly[0] = (data + (((m_gfxreg & 0x10) >> 4) * 0x100) - m_scrolly_ofs);
				break;
		}
	}
	else
	{

		switch (offset)
		{
			case 0:
				m_scrollx[1] = (data + (((m_gfxreg & 0x08) >> 3) * 0x100) - 0x1f7);
				break;
			case 1:
				m_scrolly[1] = (data + (((m_gfxreg & 0x04) >> 2) * 0x100) - 0xf9);
				break;
			case 2:
				m_scrollx[0] = (data + (((m_gfxreg & 0x20) >> 5) * 0x100) - 0x1f7);
				break;
			case 3:
				m_scrolly[0] = (data + (((m_gfxreg & 0x10) >> 4) * 0x100) - 0xf9);
				break;
		}
	}
}



/*************************************
 *
 *  Fake video controller
 *
 *************************************/

TIMER_CALLBACK_MEMBER(fromance_state::crtc_interrupt_gen)
{
	m_subcpu->set_input_line(0, HOLD_LINE);
	if (param != 0)
		m_crtc_timer->adjust(m_screen->frame_period() / param, 0, m_screen->frame_period() / param);
}

/*
 0  1  2  3  4  5
57 63 69 71 1f 00 (all games)
4f 62 69 71 1f 04 nekkyoku
 8  9  A  B
7a 7b 7d 7f  (all games)
79 7a 7d 7f  nekkyoku (gameplay/title screen)
77 79 7d 7e  nekkyoku (gals display)
 */
// TODO: guesswork, looks fully programmable
void fromance_state::crtc_refresh()
{
	if (m_gga->reg(0) == 0) // sanity check
		return;

	rectangle visarea;
	attoseconds_t refresh;

	visarea.min_x = 0;
	visarea.min_y = 0;
	visarea.max_x = ((m_gga->reg(0)+1)*4) - 1;
	visarea.max_y = 240 - 1;

	refresh = HZ_TO_ATTOSECONDS(60);

	m_screen->configure(512, 256, visarea, refresh);
}

void fromance_state::fromance_gga_data_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:
			crtc_refresh();
			break;

		case 0x0b:
			// TODO: actually is never > 0x80?
			m_crtc_timer->adjust(m_screen->time_until_vblank_start(), (data > 0x80) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", offset, data);
			break;
	}
}


/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

uint32_t fromance_state::screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx[0]);
	m_bg_tilemap->set_scrolly(0, m_scrolly[0]);
	m_fg_tilemap->set_scrollx(0, m_scrollx[1]);
	m_fg_tilemap->set_scrolly(0, m_scrolly[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
