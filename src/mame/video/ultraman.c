// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/ultraman.h"

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(ultraman_state::sprite_callback)
{
	*priority = (*color & 0x80) >> 7;
	*color = m_sprite_colorbase + ((*color & 0x7e) >> 1);
	*shadow = 0;
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(ultraman_state::zoom_callback_1)
{
	*code |= ((*color & 0x07) << 8) | (m_bank0 << 11);
	*color = m_zoom_colorbase[0] + ((*color & 0xf8) >> 3);
}

K051316_CB_MEMBER(ultraman_state::zoom_callback_2)
{
	*code |= ((*color & 0x07) << 8) | (m_bank1 << 11);
	*color = m_zoom_colorbase[1] + ((*color & 0xf8) >> 3);
}

K051316_CB_MEMBER(ultraman_state::zoom_callback_3)
{
	*code |= ((*color & 0x07) << 8) | (m_bank2 << 11);
	*color = m_zoom_colorbase[2] + ((*color & 0xf8) >> 3);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void ultraman_state::video_start()
{
	m_sprite_colorbase = 192;
	m_zoom_colorbase[0] = 0;
	m_zoom_colorbase[1] = 64;
	m_zoom_colorbase[2] = 128;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(ultraman_state::ultraman_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/*  bit 0: enable wraparound for scr #1
		    bit 1: msb of code for scr #1
		    bit 2: enable wraparound for scr #2
		    bit 3: msb of code for scr #2
		    bit 4: enable wraparound for scr #3
		    bit 5: msb of code for scr #3
		    bit 6: coin counter 1
		    bit 7: coin counter 2 */

		m_k051316_1->wraparound_enable(data & 0x01);

		if (m_bank0 != ((data & 0x02) >> 1))
		{
			m_bank0 = (data & 0x02) >> 1;
			m_k051316_1->mark_tmap_dirty();
		}

		m_k051316_2->wraparound_enable(data & 0x04);

		if (m_bank1 != ((data & 0x08) >> 3))
		{
			m_bank1 = (data & 0x08) >> 3;
			m_k051316_2->mark_tmap_dirty();
		}

		m_k051316_3->wraparound_enable(data & 0x10);

		if (m_bank2 != ((data & 0x20) >> 5))
		{
			m_bank2 = (data & 0x20) >> 5;
			m_k051316_3->mark_tmap_dirty();
		}

		coin_counter_w(machine(), 0, data & 0x40);
		coin_counter_w(machine(), 1, data & 0x80);
	}
}



/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 ultraman_state::screen_update_ultraman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k051316_3->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_k051316_2->zoom_draw(screen, bitmap, cliprect, 0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k051316_1->zoom_draw(screen, bitmap, cliprect, 0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
	return 0;
}
