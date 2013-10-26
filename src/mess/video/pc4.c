// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        VTech Laser PC4 LCD controller emulation

***************************************************************************/

#include "emu.h"
#include "includes/pc4.h"

void pc4_state::set_busy_flag(UINT16 usec)
{
	m_busy_flag = 1;
	m_busy_timer->adjust( attotime::from_usec( usec ) );
}

UINT32 pc4_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_display_on)
		for (int l=0; l<4; l++)
			for (int i=0; i<40; i++)
			{
				UINT16 char_pos = l*40 + i;

				for (int y=0; y<8; y++)
					for (int x=0; x<5; x++)
						if (m_ddram[char_pos] <= 0x10)
						{
							//draw CGRAM characters
							bitmap.pix16(l*9 + y, i*6 + x) = BIT(m_cgram[(m_ddram[char_pos]&0x07)*8+y], 4-x);
						}
						else
						{
							//draw CGROM characters
							bitmap.pix16(l*9 + y, i*6 + x) = BIT(m_region_charset->base()[m_ddram[char_pos]*8+y], 4-x);

						}

				// if is the correct position draw cursor and blink
				if (char_pos == m_cursor_pos)
				{
					//draw the cursor
					if (m_cursor_on)
						for (int x=0; x<5; x++)
							bitmap.pix16(l*9 + 7, i * 6 + x) = 1;

					if (!m_blink && m_blink_on)
						for (int y=0; y<7; y++)
							for (int x=0; x<5; x++)
								bitmap.pix16(l*9 + y, i * 6 + x) = 1;
				}
			}

	return 0;

}

void pc4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case BUSY_TIMER:
			m_busy_flag = 0;
			break;

		case BLINKING_TIMER:
			m_blink = !m_blink;
			break;
	}
}

WRITE8_MEMBER(pc4_state::lcd_control_w)
{
	if (BIT(data, 7))
	{
		m_ac_mode = 0;
		m_cursor_pos = m_ac = data & 0x7f;
		set_busy_flag(37);
	}
	else if (BIT(data, 6))
	{
		m_ac_mode = 1;
		m_ac = data & 0x3f;
		set_busy_flag(37);
	}
	else if (BIT(data, 5))
	{
		m_cursor_on = BIT(data, 3);

		set_busy_flag(37);
	}
	else if (BIT(data, 4))
	{
		UINT8 direct = (BIT(data, 2)) ? +1 : -1;

		if (BIT(data, 3))
			m_disp_shift += direct;
		else
		{
			m_ac += direct;
			m_cursor_pos += direct;
		}

		set_busy_flag(37);
	}
	else if (BIT(data, 3))
	{
		m_display_on = BIT(data, 2);
		//m_cursor_on = BIT(data, 1);
		m_blink_on = BIT(data, 0);

		set_busy_flag(37);
	}
	else if (BIT(data, 2))
	{
		m_direction = (BIT(data, 1)) ? +1 : -1;
		m_shift_on = BIT(data, 0);
		set_busy_flag(37);
	}
	else if (BIT(data, 1))
	{
		m_ac = 0;
		m_cursor_pos = 0;
		m_ac_mode = 0;
		m_direction = 1;
		m_disp_shift = 0;
		set_busy_flag(520);
	}
	else if (BIT(data, 0))
	{
		m_ac = 0;
		m_cursor_pos = 0;
		m_ac_mode = 0;
		m_direction = 1;
		m_disp_shift = 0;
		memset(m_ddram, 0x20, sizeof(m_ddram));
		set_busy_flag(520);
	}
}


READ8_MEMBER(pc4_state::lcd_control_r)
{
	return m_busy_flag<<7 || m_ac&0x7f;
}

void pc4_state::update_ac(void)
{
	int new_ac = m_ac + m_direction;
	m_ac = (new_ac < 0) ? 0 : ((new_ac >= 0xa0) ? 0xa0 : new_ac);

	if (m_ac_mode == 0)
	{
		m_cursor_pos = m_ac;
		// display is shifted only after a write
		if (m_shift_on && m_data_bus_flag == 1) m_disp_shift += m_direction;
	}

	m_data_bus_flag = 0;
}

WRITE8_MEMBER( pc4_state::lcd_offset_w )
{
	m_cursor_pos = m_ac = (data%0xa0);
}


WRITE8_MEMBER(pc4_state::lcd_data_w)
{
	if (m_ac_mode == 0)
		m_ddram[m_ac] = data;
	else
		m_cgram[m_ac] = data;

	update_ac();
	set_busy_flag(41);
}

READ8_MEMBER(pc4_state::lcd_data_r)
{
	UINT8 data;

	if (m_ac_mode == 0)
		data = m_ddram[m_ac];
	else
		data = m_cgram[m_ac];

	m_data_bus_flag = 2;
	update_ac();
	set_busy_flag(41);

	return data;
}
