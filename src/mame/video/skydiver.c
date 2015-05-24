// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sky Diver hardware

***************************************************************************/

#include "emu.h"
#include "includes/skydiver.h"
#include "sound/discrete.h"


void skydiver_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* reset all latches */
	start_lamp_1_w(space, 0, 0);
	start_lamp_2_w(space, 0, 0);
	lamp_s_w(space, 0, 0);
	lamp_k_w(space, 0, 0);
	lamp_y_w(space, 0, 0);
	lamp_d_w(space, 0, 0);
	output_set_value("lampi", 0);
	output_set_value("lampv", 0);
	output_set_value("lampe", 0);
	output_set_value("lampr", 0);
	width_w(space, 0, 0);
	coin_lockout_w(space, 0, 0);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(skydiver_state::get_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, code & 0x3f, code >> 6, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void skydiver_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skydiver_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	save_item(NAME(m_nmion));
	save_item(NAME(m_width));
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(skydiver_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


READ8_MEMBER(skydiver_state::wram_r)
{
	return m_videoram[offset | 0x380];
}

WRITE8_MEMBER(skydiver_state::wram_w)
{
	m_videoram[offset | 0x0380] = data;
}


WRITE8_MEMBER(skydiver_state::width_w)
{
	m_width = offset;
}


WRITE8_MEMBER(skydiver_state::coin_lockout_w)
{
	coin_lockout_global_w(machine(), !offset);
}


WRITE8_MEMBER(skydiver_state::start_lamp_1_w)
{
	set_led_status(machine(), 0, offset);
}

WRITE8_MEMBER(skydiver_state::start_lamp_2_w)
{
	set_led_status(machine(), 1, offset);
}


WRITE8_MEMBER(skydiver_state::lamp_s_w)
{
	output_set_value("lamps", offset);
}

WRITE8_MEMBER(skydiver_state::lamp_k_w)
{
	output_set_value("lampk", offset);
}

WRITE8_MEMBER(skydiver_state::lamp_y_w)
{
	output_set_value("lampy", offset);
}

WRITE8_MEMBER(skydiver_state::lamp_d_w)
{
	output_set_value("lampd", offset);
}

WRITE8_MEMBER(skydiver_state::_2000_201F_w)
{
	int bit = offset & 0x01;

	watchdog_reset_w(space,0,0);

	switch (offset & 0x0e)
	{
		case (0x02):
			output_set_value("lampi", bit);
			break;
		case (0x04):
			output_set_value("lampv", bit);
			break;
		case (0x06):
			output_set_value("lampe", bit);
			break;
		case (0x08):
			output_set_value("lampr", bit);
			break;
		case (0x0a):
			m_discrete->write(space, SKYDIVER_OCT1_EN, bit);
			break;
		case (0x0c):
			m_discrete->write(space, SKYDIVER_OCT2_EN, bit);
			break;
		case (0x0e):
			m_discrete->write(space, SKYDIVER_NOISE_RST, bit);
			break;
	}
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


UINT32 skydiver_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	draw_sprites(bitmap, cliprect);
	return 0;
}
