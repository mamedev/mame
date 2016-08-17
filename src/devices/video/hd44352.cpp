// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Hitachi HD44352 LCD controller

***************************************************************************/

#include "emu.h"
#include "video/hd44352.h"

#define     LCD_BYTE_INPUT          0x01
#define     LCD_BYTE_OUTPUT         0x02
#define     LCD_CHAR_OUTPUT         0x03
#define     LCD_ON_OFF              0x04
#define     LCD_CURSOR_GRAPHIC      0x06
#define     LCD_CURSOR_CHAR         0x07
#define     LCD_SCROLL_CHAR_WIDTH   0x08
#define     LCD_CURSOR_STATUS       0x09
#define     LCD_USER_CHARACTER      0x0b
#define     LCD_CONTRAST            0x0c
#define     LCD_IRQ_FREQUENCY       0x0d
#define     LCD_CURSOR_POSITION     0x0e


// devices
const device_type HD44352 = &device_creator<hd44352_device>;

//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  hd44352_device - constructor
//-------------------------------------------------

hd44352_device::hd44352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock):
	device_t(mconfig, HD44352, "hd44352", tag, owner, clock, "hd44352", __FILE__),
	m_on_cb(*this),
	m_char_rom(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void hd44352_device::device_validity_check(validity_checker &valid) const
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd44352_device::device_start()
{
	m_on_cb.resolve_safe();

	m_on_timer = timer_alloc(ON_TIMER);
	m_on_timer->adjust(attotime::from_hz(m_clock/16384), 0, attotime::from_hz(m_clock/16384));

	save_item( NAME(m_control_lines));
	save_item( NAME(m_data_bus));
	save_item( NAME(m_state));
	save_item( NAME(m_offset));
	save_item( NAME(m_char_width));
	save_item( NAME(m_bank));
	save_item( NAME(m_lcd_on));
	save_item( NAME(m_scroll));
	save_item( NAME(m_contrast));
	save_item( NAME(m_byte_count));
	save_item( NAME(m_cursor_status));
	save_item( NAME(m_cursor_x));
	save_item( NAME(m_cursor_y));
	save_item( NAME(m_cursor_lcd));
	save_item( NAME(m_video_ram[0]));
	save_item( NAME(m_video_ram[1]));
	save_item( NAME(m_par));
	save_item( NAME(m_cursor));
	save_item( NAME(m_custom_char[0]));
	save_item( NAME(m_custom_char[1]));
	save_item( NAME(m_custom_char[2]));
	save_item( NAME(m_custom_char[3]));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd44352_device::device_reset()
{
	memset(m_video_ram, 0x00, sizeof(m_video_ram));
	memset(m_par, 0x00, sizeof(m_par));
	memset(m_custom_char, 0x00, sizeof(m_custom_char));
	memset(m_cursor, 0x00, sizeof(m_cursor));
	m_control_lines = 0;
	m_data_bus = 0xff;
	m_state = 0;
	m_bank = 0;
	m_offset = 0;
	m_char_width = 6;
	m_lcd_on = 0;
	m_scroll = 0;
	m_byte_count = 0;
	m_cursor_status = 0;
	m_cursor_x = 0;
	m_cursor_y = 0;
	m_cursor_lcd = 0;
	m_contrast = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void hd44352_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case ON_TIMER:
			if (m_control_lines & 0x40)
			{
				m_on_cb(ASSERT_LINE);
				m_on_cb(CLEAR_LINE);
			}
			break;
	}
}

//**************************************************************************
//  device interface
//**************************************************************************

UINT32 hd44352_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 cw = m_char_width;

	bitmap.fill(0, cliprect);

	if (m_control_lines&0x80 && m_lcd_on)
	{
		for (int a=0; a<2; a++)
			for (int py=0; py<4; py++)
				for (int px=0; px<16; px++)
					if (BIT(m_cursor_status, 4) && px == m_cursor_x && py == m_cursor_y && a == m_cursor_lcd)
					{
						//draw the cursor
						for (int c=0; c<cw; c++)
						{
							UINT8 d = compute_newval((m_cursor_status>>5) & 0x07, m_video_ram[a][py*16*cw + px*cw + c + m_scroll * 48], m_cursor[c]);
							for (int b=0; b<8; b++)
							{
								bitmap.pix16(py*8 + b, a*cw*16 + px*cw + c) = BIT(d, 7-b);
							}
						}
					}
					else
					{
						for (int c=0; c<cw; c++)
						{
							UINT8 d = m_video_ram[a][py*16*cw + px*cw + c + m_scroll * 48];
							for (int b=0; b<8; b++)
							{
								bitmap.pix16(py*8 + b, a*cw*16 + px*cw + c) = BIT(d, 7-b);
							}
						}
					}
	}

	return 0;
}


void hd44352_device::control_write(UINT8 data)
{
	if(m_control_lines == data)
		m_state = 0;

	m_control_lines = data;
}

UINT8 hd44352_device::compute_newval(UINT8 type, UINT8 oldval, UINT8 newval)
{
	switch(type & 0x07)
	{
		case 0x00:
			return (~oldval) & newval;
		case 0x01:
			return oldval ^ newval;
		case 0x03:
			return oldval & (~newval);
		case 0x04:
			return newval;
		case 0x05:
			return oldval | newval;
		case 0x07:
			return oldval;
		case 0x02:
		case 0x06:
		default:
			return 0;
	}
}

UINT8 hd44352_device::get_char(UINT16 pos)
{
	switch ((UINT8)pos/8)
	{
		case 0xcf:
			return m_custom_char[0][pos%8];
		case 0xdf:
			return m_custom_char[1][pos%8];
		case 0xef:
			return m_custom_char[2][pos%8];
		case 0xff:
			return m_custom_char[3][pos%8];
		default:
			return m_char_rom[pos];
	}
}

void hd44352_device::data_write(UINT8 data)
{
	// verify that controller is active
	if (!(m_control_lines&0x80))
		return;

	if (m_control_lines & 0x01)
	{
		if (!(m_control_lines&0x02) && !(m_control_lines&0x04))
			return;

		switch (m_state)
		{
			case 0:     //parameter 0
				m_par[m_state++] = data;
				break;
			case 1:     //parameter 1
				m_par[m_state++] = data;
				break;
			case 2:     //parameter 2
				m_par[m_state++] = data;
				break;
		}

		switch (m_par[0] & 0x0f)
		{
			case LCD_BYTE_INPUT:
			case LCD_CHAR_OUTPUT:
				{
					if (m_state == 1)
						m_bank = BIT(data, 4);
					else if (m_state == 2)
						m_offset = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
					else if (m_state == 3)
						m_offset += ((data & 0x03) * 96);
				}
				break;
			case LCD_BYTE_OUTPUT:
				{
					if (m_state == 1)
						m_bank = BIT(data, 4);
					else if (m_state == 2)
						m_offset = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
					else if (m_state == 3)
						m_offset += ((data & 0x03) * 96);
				}
				break;
			case LCD_ON_OFF:
				{
					if (m_state == 1)
						m_lcd_on = BIT(data, 4);
					m_data_bus = 0xff;
					m_state = 0;
				}
				break;
			case LCD_SCROLL_CHAR_WIDTH:
				{
					if (m_state == 1)
					{
						m_char_width = 8-((data>>4)&3);
						m_scroll = ((data>>6)&3);
					}

					m_data_bus = 0xff;
					m_state = 0;
				}
				break;
			case LCD_CURSOR_STATUS:
				{
					if (m_state == 1)
						m_cursor_status = data;
					m_data_bus = 0xff;
					m_state = 0;
				}
				break;
			case LCD_CONTRAST:
				{
					if (m_state == 1)
						m_contrast = (m_contrast & 0x00ffff) | (data<<16);
					else if (m_state == 2)
						m_contrast = (m_contrast & 0xff00ff) | (data<<8);
					else if (m_state == 3)
					{
						m_contrast = (m_contrast & 0xffff00) | (data<<0);
						m_state = 0;
					}

					m_data_bus = 0xff;
				}
				break;
			case LCD_IRQ_FREQUENCY:
				{
					if (m_state == 1)
					{
						UINT32 on_timer_rate;

						switch((data>>4) & 0x0f)
						{
							case 0x00:      on_timer_rate = 16384;      break;
							case 0x01:      on_timer_rate = 8;          break;
							case 0x02:      on_timer_rate = 16;         break;
							case 0x03:      on_timer_rate = 32;         break;
							case 0x04:      on_timer_rate = 64;         break;
							case 0x05:      on_timer_rate = 128;        break;
							case 0x06:      on_timer_rate = 256;        break;
							case 0x07:      on_timer_rate = 512;        break;
							case 0x08:      on_timer_rate = 1024;       break;
							case 0x09:      on_timer_rate = 2048;       break;
							case 0x0a:      on_timer_rate = 4096;       break;
							case 0x0b:      on_timer_rate = 4096;       break;
							default:        on_timer_rate = 8192;       break;
						}

						m_on_timer->adjust(attotime::from_hz(m_clock/on_timer_rate), 0, attotime::from_hz(m_clock/on_timer_rate));
					}
					m_data_bus = 0xff;
					m_state = 0;
				}
				break;
			case LCD_CURSOR_POSITION:
				{
					if (m_state == 1)
						m_cursor_lcd = BIT(data, 4);    //0:left lcd 1:right lcd;
					else if (m_state == 2)
						m_cursor_x = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
					else if (m_state == 3)
					{
						m_cursor_y = data & 0x03;
						m_state = 0;
					}

					m_data_bus = 0xff;
				}
				break;
		}

		m_byte_count = 0;
		m_data_bus = 0xff;
	}
	else
	{
		switch (m_par[0] & 0x0f)
		{
			case LCD_BYTE_INPUT:
				{
					if (((m_par[0]>>5) & 0x07) != 0x03)
						break;

					m_offset %= 0x180;
					m_data_bus = ((m_video_ram[m_bank][m_offset]<<4)&0xf0) | ((m_video_ram[m_bank][m_offset]>>4)&0x0f);
					m_offset++; m_byte_count++;
				}
				break;
			case LCD_BYTE_OUTPUT:
				{
					m_offset %= 0x180;
					m_video_ram[m_bank][m_offset] = compute_newval((m_par[0]>>5) & 0x07, m_video_ram[m_bank][m_offset], data);
					m_offset++; m_byte_count++;

					m_data_bus = 0xff;
				}
				break;
			case LCD_CHAR_OUTPUT:
				{
					int char_pos = data*8;

					for (int i=0; i<m_char_width; i++)
					{
						m_offset %= 0x180;
						m_video_ram[m_bank][m_offset] = compute_newval((m_par[0]>>5) & 0x07, m_video_ram[m_bank][m_offset], get_char(char_pos));
						m_offset++; char_pos++;
					}

					m_byte_count++;
					m_data_bus = 0xff;
				}
				break;
			case LCD_CURSOR_GRAPHIC:
				if (m_byte_count<8)
				{
					m_cursor[m_byte_count] = data;
					m_byte_count++;
					m_data_bus = 0xff;
				}
				break;
			case LCD_CURSOR_CHAR:
				if (m_byte_count<1)
				{
					UINT8 char_code = ((data<<4)&0xf0) | ((data>>4)&0x0f);

					for (int i=0; i<8; i++)
						m_cursor[i] = get_char(char_code*8 + i);

					m_byte_count++;
					m_data_bus = 0xff;
				}
				break;
			case LCD_USER_CHARACTER:
				if (m_byte_count<8)
				{
					m_custom_char[(m_par[1]&0x03)][m_byte_count] = data;
					m_byte_count++;
					m_data_bus = 0xff;
				}
				break;
			default:
				m_data_bus = 0xff;
		}

		m_state=0;
	}
}

UINT8 hd44352_device::data_read()
{
	return m_data_bus;
}
