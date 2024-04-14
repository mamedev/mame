// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Toshiba T6A04 LCD controller

        TODO:
        - busy flag
        - contrast
        - slave mode

***************************************************************************/

#include "emu.h"
#include "t6a04.h"

// devices
DEFINE_DEVICE_TYPE(T6A04, t6a04_device, "t6a04", "Toshiba T6A04 LCD Controller")

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void t6a04_device::device_validity_check(validity_checker &valid) const
{
	if (m_height == 0 || m_width == 0)
		osd_printf_error("Configured with invalid parameter\n");
}

//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  t6a04_device - constructor
//-------------------------------------------------

t6a04_device::t6a04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, T6A04, tag, owner, clock),
	m_busy_flag(0), m_display_on(0), m_contrast(0),
	m_xpos(0), m_ypos(0), m_zpos(0),
	m_direction(0), m_active_counter(0), m_word_len(0),
	m_opa1(0), m_opa2(0), m_output_reg(0),
	m_height(0), m_width(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6a04_device::device_start()
{
	save_item(NAME(m_busy_flag));
	save_item(NAME(m_display_on));
	save_item(NAME(m_contrast));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_zpos));
	save_item(NAME(m_active_counter));
	save_item(NAME(m_direction));
	save_item(NAME(m_word_len));
	save_item(NAME(m_opa1));
	save_item(NAME(m_opa2));
	save_item(NAME(m_output_reg));
	save_item(NAME(m_lcd_ram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void t6a04_device::device_reset()
{
	//values taken from the datasheet
	memset(m_lcd_ram, 0x00, sizeof(m_lcd_ram));
	m_busy_flag = 0;
	m_display_on = 0;
	m_contrast = 0;
	m_xpos = 0;
	m_ypos = 0;
	m_zpos = 0;
	m_direction = 1;
	m_active_counter = 1;
	m_word_len = 1; //8bit mode
	m_opa1 = 0;
	m_opa2 = 0;
	m_output_reg = 0;
}


//**************************************************************************
//  device interface
//**************************************************************************

uint32_t t6a04_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t ypages = m_width>>3;
	uint8_t last_line = m_zpos + m_height;

	if (m_display_on)
	{
		for (int y=0; y<ypages; y++)
			for (int x=m_zpos; x<last_line; x++)
			{
				uint8_t data = m_lcd_ram[(x&0x3f)*15 + y];

				for (int b=7; b>=0; b--)
				{
					bitmap.pix(x&0x3f, y*8+b) = data & 1;
					data>>=1;
				}
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	return 0;
}

void t6a04_device::control_write(uint8_t data)
{
	if ((data & 0xc0) == 0xc0) // SCE (set contrast)
	{
		m_contrast = data&0x3f;
	}
	else if ((data & 0xc0) == 0x80) // SXE (set x address)
	{
		m_xpos = data&0x3f;
	}
	else if ((data & 0xc0) == 0x40) // SZE (set z address)
	{
		m_zpos = data&0x3f;
	}
	else if ((data & 0xe0) == 0x20) // SYE (set y address)
	{
		m_ypos = data&0x1f;
	}
	else if ((data & 0xf8) == 0x18) // CHE (test mode)
	{
		//???
	}
	else if ((data & 0xf8) == 0x10) // OPA1 (op-amp control 1)
	{
		m_opa1 = data & 3;
	}
	else if ((data & 0xf8) == 0x08) // OPA2 (op-amp control 2)
	{
		m_opa2 = data & 3;
	}
	else if ((data & 0xfc) == 0x04) // UDE (up/down mode)
	{
		m_active_counter = (data & 0x02) >> 1;
		m_direction = (data & 0x01) ? +1 : -1;
	}
	else if ((data & 0xfe) == 0x02) // DPE (display on/off)
	{
		m_display_on = data & 1;
	}
	else if ((data & 0xfe) == 0x00) // 86E (word length)
	{
		m_word_len = data & 1;
	}
}

uint8_t t6a04_device::control_read()
{
	/*
	    status read
	    x--- ----   busy
	    -x-- ----   8/6 mode
	    --x- ----   display on/off
	    ---x ----   reset state
	    ---- xx--   unused (always 0)
	    ---- --x-   x/y counter
	    ---- ---x   up/down mode
	*/

	return (m_busy_flag<<7) | (m_word_len<<6) | (m_display_on<<5) | (m_active_counter<<1) | (m_direction == 1 ? 1 : 0);
}

void t6a04_device::data_write(uint8_t data)
{
	if (m_word_len)
	{
		//8bit mode
		m_lcd_ram[m_xpos*15 + m_ypos] = data;
	}
	else
	{
		//6bit mode
		data = data<<0x02;
		uint8_t start_bit = m_ypos * 6;
		uint8_t pos_bit = start_bit & 0x07;
		uint8_t *ti82_video = &m_lcd_ram[(m_xpos*15)+(start_bit>>3)];

		ti82_video[0] = (ti82_video[0] & ~(0xFC>>pos_bit)) | (data>>pos_bit);
		if(pos_bit>0x02)
			ti82_video[1] = (ti82_video[1] & ~(0xFC<<(8-pos_bit))) | (data<<(8-pos_bit));
	}

	if (m_active_counter)
		m_ypos = (m_ypos + m_direction) & 0x1f;
	else
		m_xpos = (m_xpos + m_direction) & 0x3f;

}

uint8_t t6a04_device::data_read()
{
	uint8_t data = m_output_reg;
	uint8_t output_reg;

	if (m_word_len)
	{
		//8bit mode
		output_reg = m_lcd_ram[m_xpos*15 + m_ypos];
	}
	else
	{
		//6bit mode
		uint8_t start_bit = m_ypos * 6;
		uint8_t pos_bit = start_bit & 7;
		uint8_t *ti82_video = &m_lcd_ram[(m_xpos*15)+(start_bit>>3)];

		output_reg = ((((*ti82_video)<<8)+ti82_video[1])>>(10-pos_bit));
	}

	if (!machine().side_effects_disabled())
	{
		m_output_reg = output_reg;

		if (m_active_counter)
			m_ypos = (m_ypos + m_direction) & 0x1f;
		else
			m_xpos = (m_xpos + m_direction) & 0x3f;
	}

	return data;
}
