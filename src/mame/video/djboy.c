// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/**
 * @file video/djboy.c
 *
 * video hardware for DJ Boy
 */
#include "emu.h"
#include "video/kan_pand.h"
#include "includes/djboy.h"

WRITE8_MEMBER(djboy_state::djboy_scrollx_w)
{
	m_scrollx = data;
}

WRITE8_MEMBER(djboy_state::djboy_scrolly_w)
{
	m_scrolly = data;
}

TILE_GET_INFO_MEMBER(djboy_state::get_bg_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x800];
	int code = m_videoram[tile_index] + (attr & 0xf) * 256;
	int color = attr >> 4;

	if (color & 8)
		code |= 0x1000;

	SET_TILE_INFO_MEMBER(1, code, color, 0);    /* no flip */
}

WRITE8_MEMBER(djboy_state::djboy_videoram_w)
{
	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset & 0x7ff);
}

void djboy_state::video_start()
{
	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(djboy_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
}

WRITE8_MEMBER(djboy_state::djboy_paletteram_w)
{
	int val;

	m_paletteram[offset] = data;
	offset &= ~1;
	val = (m_paletteram[offset] << 8) | m_paletteram[offset + 1];

	m_palette->set_pen_color(offset / 2, pal4bit(val >> 8), pal4bit(val >> 4), pal4bit(val >> 0));
}

UINT32 djboy_state::screen_update_djboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/**
	 * xx------ msb x
	 * --x----- msb y
	 * ---x---- flipscreen?
	 * ----xxxx ROM bank
	 */
	int scroll;

	scroll = m_scrollx | ((m_videoreg & 0xc0) << 2);
	m_background->set_scrollx(0, scroll - 0x391);

	scroll = m_scrolly | ((m_videoreg & 0x20) << 3);
	m_background->set_scrolly(0, scroll);

	m_background->draw(screen, bitmap, cliprect, 0, 0);
	m_pandora->update(bitmap, cliprect);

	return 0;
}

void djboy_state::screen_eof_djboy(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_pandora->eof();
	}
}
