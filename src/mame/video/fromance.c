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

	SET_TILE_INFO_MEMBER(layer, tile, color, 0);
}

TILE_GET_INFO_MEMBER(fromance_state::get_fromance_bg_tile_info){ get_fromance_tile_info(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(fromance_state::get_fromance_fg_tile_info){ get_fromance_tile_info(tileinfo, tile_index, 1); }


inline void fromance_state::get_nekkyoku_tile_info( tile_data &tileinfo, int tile_index, int layer )
{
	int tile = (m_local_videoram[layer][0x0000 + tile_index] << 8) |
				m_local_videoram[layer][0x1000 + tile_index];
	int color = m_local_videoram[layer][tile_index + 0x2000] & 0x3f;

	SET_TILE_INFO_MEMBER(layer, tile, color, 0);
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
	m_local_videoram[0] = auto_alloc_array(machine(), UINT8, 0x1000 * 3);
	m_local_videoram[1] = auto_alloc_array(machine(), UINT8, 0x1000 * 3);

	/* allocate local palette RAM */
	m_local_paletteram = auto_alloc_array(machine(), UINT8, 0x800 * 2);

	/* configure tilemaps */
	m_fg_tilemap->set_transparent_pen(15);

	/* reset the timer */
	m_crtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(fromance_state::crtc_interrupt_gen),this));

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
	save_item(NAME(m_crtc_register));
	save_item(NAME(m_crtc_data));
	save_pointer(NAME(m_local_paletteram), 0x800 * 2);
}

VIDEO_START_MEMBER(fromance_state,fromance)
{
	/* allocate tilemaps */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fromance_state::get_fromance_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fromance_state::get_fromance_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);

	init_common();
}

VIDEO_START_MEMBER(fromance_state,nekkyoku)
{
	/* allocate tilemaps */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fromance_state::get_nekkyoku_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fromance_state::get_nekkyoku_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);

	init_common();
}

VIDEO_START_MEMBER(fromance_state,pipedrm)
{
	VIDEO_START_CALL_MEMBER(fromance);
	m_scrolly_ofs = 0x00;
}

VIDEO_START_MEMBER(fromance_state,hatris)
{
	VIDEO_START_CALL_MEMBER(fromance);
	m_scrollx_ofs = 0xB9;
	m_scrolly_ofs = 0x00;
}

/*************************************
 *
 *  Graphics control register
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_gfxreg_w)
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

READ8_MEMBER(fromance_state::fromance_paletteram_r)
{
	/* adjust for banking and read */
	offset |= m_selected_paletteram << 11;
	return m_local_paletteram[offset];
}


WRITE8_MEMBER(fromance_state::fromance_paletteram_w)
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

READ8_MEMBER(fromance_state::fromance_videoram_r)
{
	return m_local_videoram[m_selected_videoram][offset];
}


WRITE8_MEMBER(fromance_state::fromance_videoram_w)
{
	m_local_videoram[m_selected_videoram][offset] = data;
	(m_selected_videoram ? m_fg_tilemap : m_bg_tilemap)->mark_tile_dirty(offset & 0x0fff);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_scroll_w)
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


WRITE8_MEMBER(fromance_state::fromance_crtc_data_w)
{
	m_crtc_data[m_crtc_register] = data;

	switch (m_crtc_register)
	{
		/* only register we know about.... */
		case 0x0b:
			m_crtc_timer->adjust(m_screen->time_until_vblank_start(), (data > 0x80) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", m_crtc_register, data & 0xff);
			break;
	}
}


WRITE8_MEMBER(fromance_state::fromance_crtc_register_w)
{
	m_crtc_register = data;
}





/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

UINT32 fromance_state::screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx[0]);
	m_bg_tilemap->set_scrolly(0, m_scrolly[0]);
	m_fg_tilemap->set_scrollx(0, m_scrollx[1]);
	m_fg_tilemap->set_scrolly(0, m_scrolly[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 fromance_state::screen_update_pipedrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8* sram = m_spriteram;

	/* there seems to be no logical mapping for the X scroll register -- maybe it's gone */
	m_bg_tilemap->set_scrolly(0, m_scrolly[1]);
	m_fg_tilemap->set_scrolly(0, m_scrolly[0]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_spr_old->turbofrc_draw_sprites((UINT16*)sram, m_spriteram.bytes(), 0, bitmap, cliprect, screen.priority(), 0);
	m_spr_old->turbofrc_draw_sprites((UINT16*)sram, m_spriteram.bytes(), 0, bitmap, cliprect, screen.priority(), 1);
	return 0;
}
