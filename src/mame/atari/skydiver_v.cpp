// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sky Diver hardware

***************************************************************************/

#include "emu.h"
#include "skydiver.h"
#include "sound/discrete.h"


void skydiver_state::machine_reset()
{
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(skydiver_state::get_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	tileinfo.set(0, code & 0x3f, code >> 6, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void skydiver_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skydiver_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	save_item(NAME(m_nmion));
	save_item(NAME(m_width));

	m_leds.resolve();
	m_lamp_s.resolve();
	m_lamp_k.resolve();
	m_lamp_y.resolve();
	m_lamp_d.resolve();
	m_lamp_i.resolve();
	m_lamp_v.resolve();
	m_lamp_e.resolve();
	m_lamp_r.resolve();
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void skydiver_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t skydiver_state::wram_r(offs_t offset)
{
	return m_videoram[offset | 0x380];
}

void skydiver_state::wram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset | 0x0380] = data;
}


WRITE_LINE_MEMBER(skydiver_state::width_w)
{
	m_width = state;
}


WRITE_LINE_MEMBER(skydiver_state::coin_lockout_w)
{
	machine().bookkeeping().coin_lockout_global_w(!state);
}


WRITE_LINE_MEMBER(skydiver_state::start_lamp_1_w)
{
	m_leds[0] = state;
}

WRITE_LINE_MEMBER(skydiver_state::start_lamp_2_w)
{
	m_leds[1] = state;
}


WRITE_LINE_MEMBER(skydiver_state::lamp_s_w)
{
	m_lamp_s = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_k_w)
{
	m_lamp_k = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_y_w)
{
	m_lamp_y = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_d_w)
{
	m_lamp_d = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_i_w)
{
	m_lamp_i = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_v_w)
{
	m_lamp_v = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_e_w)
{
	m_lamp_e = state;
}

WRITE_LINE_MEMBER(skydiver_state::lamp_r_w)
{
	m_lamp_r = state;
}

void skydiver_state::latch3_watchdog_w(offs_t offset, uint8_t data)
{
	m_watchdog->watchdog_reset();
	m_latch3->write_a0(offset);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

void skydiver_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pic;


	/* draw each one of our four motion objects, the two PLANE sprites
	   can be drawn double width */
	for (pic = 3; pic >= 0; pic--)
	{
		int sx,sy;
		int charcode;
		int xflip, yflip;
		int color;
		int wide;

		sx = 29*8 - m_videoram[pic + 0x0390];
		sy = 30*8 - m_videoram[pic*2 + 0x0398];
		charcode = m_videoram[pic*2 + 0x0399];
		xflip = charcode & 0x10;
		yflip = charcode & 0x08;
		wide = (~pic & 0x02) && m_width;
		charcode = (charcode & 0x07) | ((charcode & 0x60) >> 2);
		color = pic & 0x01;

		if (wide)
		{
			sx -= 8;
		}

		m_gfxdecode->gfx(1)->zoom_transpen(bitmap,cliprect,
			charcode, color,
			xflip,yflip,sx,sy,
			wide ? 0x20000 : 0x10000, 0x10000,0);
	}
}


uint32_t skydiver_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	draw_sprites(bitmap, cliprect);
	return 0;
}
