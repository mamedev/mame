// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert

#include "includes/gamecom.h"

#define Y_PIXELS 200


TIMER_CALLBACK_MEMBER(gamecom_state::gamecom_scanline)
	{
	// draw line
	m_base_address = ( m_p_ram[SM8521_LCDC] & 0x40 ) ? 0x2000 : 0x0000;

	if ( ~m_p_ram[SM8521_LCDC] & 0x80 )
	{
		rectangle rec(0, Y_PIXELS - 1, m_scanline, m_scanline);
		m_bitmap.fill(0, rec );
		return;
	}
	else
	{
		UINT8 *line = &m_p_videoram[ m_base_address + 40 * m_scanline ];
		int pal[4];
		int i;

		switch( m_p_ram[SM8521_LCDC] & 0x30 )
		{
		case 0x00:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 2;
			pal[3] = 0;
			break;
		case 0x10:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 1;
			pal[3] = 0;
			break;
		case 0x20:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 1;
			pal[3] = 0;
			break;
		case 0x30:
			pal[0] = 4;
			pal[1] = 2;
			pal[2] = 1;
			pal[3] = 0;
			break;
		}
		for( i = 0; i < 40; i++ )
		{
			UINT8 p = line[i];
			m_bitmap.pix16(i * 4 + 0, m_scanline) = pal[ ( p >> 6 ) & 3 ];
			m_bitmap.pix16(i * 4 + 1, m_scanline) = pal[ ( p >> 4 ) & 3 ];
			m_bitmap.pix16(i * 4 + 2, m_scanline) = pal[ ( p >> 2 ) & 3 ];
			m_bitmap.pix16(i * 4 + 3, m_scanline) = pal[ ( p      ) & 3 ];
		}
	}

	m_scanline = ( m_scanline + 1 ) % Y_PIXELS;
}

void gamecom_state::video_start()
{
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamecom_state::gamecom_scanline),this));
	m_scanline_timer->adjust( machine().first_screen()->time_until_pos(0 ), 0, machine().first_screen()->scan_period() );
	machine().first_screen()->register_screen_bitmap(m_bitmap);
}
