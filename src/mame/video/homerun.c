// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "emu.h"
#include "includes/homerun.h"


/**************************************************************************/

CUSTOM_INPUT_MEMBER(homerun_state::homerun_sprite0_r)
{
	// sprite-0 vs background collision status, similar to NES
	return (m_screen->vpos() > (m_spriteram[0] - 16 + 1)) ? 1 : 0;
}

WRITE8_MEMBER(homerun_state::homerun_scrollhi_w)
{
	// d0: scroll y high bit
	// d1: scroll x high bit
	// other bits: ?
	m_scrolly = (m_scrolly & 0xff) | (data << 8 & 0x100);
	m_scrollx = (m_scrollx & 0xff) | (data << 7 & 0x100);
}

WRITE8_MEMBER(homerun_state::homerun_scrolly_w)
{
	m_scrolly = (m_scrolly & 0xff00) | data;
}

WRITE8_MEMBER(homerun_state::homerun_scrollx_w)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

WRITE8_MEMBER(homerun_state::homerun_banking_w)
{
	// games do mid-screen gfx bank switching
	int vpos = m_screen->vpos();
	m_screen->update_partial(vpos);

	// d0-d1: gfx bank
	// d2-d4: ?
	// d5-d7: prg bank
	m_gfx_ctrl = data;
	m_tilemap->mark_all_dirty();
	membank("bank1")->set_entry(data >> 5 & 7);
}

WRITE8_MEMBER(homerun_state::homerun_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(homerun_state::homerun_color_w)
{
	m_colorram[offset] = data;

	/* from PCB photo:
	    bit 7:  470 ohm resistor \
	    bit 6:  220 ohm resistor -  --> 470 ohm resistor  --> blue
	    bit 5:  470 ohm resistor \
	    bit 4:  220 ohm resistor -  --> 470 ohm resistor  --> green
	    bit 3:  1  kohm resistor /
	    bit 2:  470 ohm resistor \
	    bit 1:  220 ohm resistor -  --> 470 ohm resistor  --> red
	    bit 0:  1  kohm resistor /
	*/

	// let's implement it the old fashioned way until it's found out how exactly the resnet is hooked up
	int r, g, b;
	int bit0, bit1, bit2;

	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = 0;
	bit1 = (data >> 6) & 0x01;
	bit2 = (data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	m_palette->set_pen_color(offset, rgb_t(r,g,b));
}


/**************************************************************************/

TILE_GET_INFO_MEMBER(homerun_state::get_homerun_tile_info)
{
	int tileno = (m_videoram[tile_index]) | ((m_videoram[tile_index | 0x1000] & 0x38) << 5) | ((m_gfx_ctrl & 1) << 11);
	int palno = (m_videoram[tile_index | 0x1000] & 0x07);

	SET_TILE_INFO_MEMBER(0, tileno, palno, 0);
}


void homerun_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homerun_state::get_homerun_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
}


void homerun_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (spriteram[offs + 0] == 0)
			continue;

		int sy = spriteram[offs + 0] - 16 + 1;
		int sx = spriteram[offs + 3];
		int code = (spriteram[offs + 1]) | ((spriteram[offs + 2] & 0x8) << 5) | ((m_gfx_ctrl & 3) << 9);
		int color = (spriteram[offs + 2] & 0x07) | 8;
		int flipx = (spriteram[offs + 2] & 0x40) >> 6;
		int flipy = (spriteram[offs + 2] & 0x80) >> 7;

		if (sy >= 0)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);

			// wraparound x
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx - 256 , sy, 0);
		}
	}
}

UINT32 homerun_state::screen_update_homerun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrolly(0, m_scrolly);
	m_tilemap->set_scrollx(0, m_scrollx);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}
