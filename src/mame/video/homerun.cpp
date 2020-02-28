// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "emu.h"
#include "includes/homerun.h"


/**************************************************************************/

READ_LINE_MEMBER(homerun_state::sprite0_r)
{
	// sprite-0 vs background collision status, similar to NES
	return (m_screen->vpos() > (m_spriteram[0] - 16 + 1)) ? 1 : 0;
}

WRITE8_MEMBER(homerun_state::scrollhi_w)
{
	// d0: scroll y high bit
	// d1: scroll x high bit
	// other bits: ?
	m_scrolly = (m_scrolly & 0xff) | (data << 8 & 0x100);
	m_scrollx = (m_scrollx & 0xff) | (data << 7 & 0x100);
}

WRITE8_MEMBER(homerun_state::scrolly_w)
{
	m_scrolly = (m_scrolly & 0xff00) | data;
}

WRITE8_MEMBER(homerun_state::scrollx_w)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

void homerun_state::banking_w(u8 data)
{
	u8 const old = m_gfx_ctrl;
	if (old ^ data)
	{
		if ((old ^ data) & 3)
		{
			// games do mid-screen gfx bank switching
			int vpos = m_screen->vpos();
			m_screen->update_partial(vpos);
		}

		// d0-d1: gfx bank
		// d2-d4: ?
		// d5-d7: prg bank
		m_gfx_ctrl = data;
		if ((old ^ m_gfx_ctrl) & 1)
			m_tilemap->mark_all_dirty();

		if ((old ^ m_gfx_ctrl) >> 5 & 7)
			m_mainbank->set_entry(m_gfx_ctrl >> 5 & 7);

	}
}

WRITE8_MEMBER(homerun_state::videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0xfff);
}

rgb_t homerun_state::homerun_RGB332(u32 raw)
{
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
	u8 bit0, bit1, bit2;

	bit0 = (raw >> 0) & 0x01;
	bit1 = (raw >> 1) & 0x01;
	bit2 = (raw >> 2) & 0x01;
	int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = (raw >> 3) & 0x01;
	bit1 = (raw >> 4) & 0x01;
	bit2 = (raw >> 5) & 0x01;
	int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = 0;
	bit1 = (raw >> 6) & 0x01;
	bit2 = (raw >> 7) & 0x01;
	int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	return rgb_t(r, g, b);
}

/**************************************************************************/

TILE_GET_INFO_MEMBER(homerun_state::get_tile_info)
{
	u32 const tileno = (m_videoram[tile_index]) | ((m_videoram[tile_index | 0x1000] & 0x38) << 5) | ((m_gfx_ctrl & 1) << 11);
	u16 const palno = (m_videoram[tile_index | 0x1000] & 0x07);

	SET_TILE_INFO_MEMBER(0, tileno, palno, 0);
}


void homerun_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(homerun_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	save_item(NAME(m_gfx_ctrl));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_scrollx));
}


void homerun_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (m_spriteram[offs + 0] == 0)
			continue;

		int const sy     = m_spriteram[offs + 0] - 16 + 1;
		int const sx     = m_spriteram[offs + 3];
		u32 const code   = (m_spriteram[offs + 1]) | ((m_spriteram[offs + 2] & 0x8) << 5) | ((m_gfx_ctrl & 3) << 9);
		u32 const color  = (m_spriteram[offs + 2] & 0x07) | 8;
		bool const flipx = (m_spriteram[offs + 2] & 0x40) >> 6;
		bool const flipy = (m_spriteram[offs + 2] & 0x80) >> 7;

		if (sy >= 0)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);

			// wraparound x
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx - 256 , sy, 0);
		}
	}
}

u32 homerun_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrolly(0, m_scrolly);
	m_tilemap->set_scrollx(0, m_scrollx);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}
