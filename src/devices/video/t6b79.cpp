// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Toshiba T6B79 LCD controller

***************************************************************************/

#include "emu.h"
#include "t6b79.h"

DEFINE_DEVICE_TYPE(T6B79, t6b79_device, "t6b79", "Toshiba T6B79 LCD Controller")

t6b79_device::t6b79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, T6B79, tag, owner, clock),
	m_display_on(0), m_contrast(0),
	m_xpos(0), m_ypos(0), m_zpos(0),
	m_direction(0), m_active_counter(0), m_word_len(0),
	m_opa1(0), m_opa2(0), m_output_reg(0), m_stb(0),
	m_height(48), m_width(64)
{
}

void t6b79_device::device_start()
{
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
	save_item(NAME(m_stb));
	save_item(NAME(m_lcd_ram));
}

void t6b79_device::device_reset()
{
	memset(m_lcd_ram, 0x00, sizeof(m_lcd_ram));
	m_display_on = 0;
	m_contrast = 0;
	m_xpos = 0;
	m_ypos = 0;
	m_zpos = 0;
	m_direction = 1;
	m_active_counter = 1;
	m_word_len = 1;
	m_opa1 = 0;
	m_opa2 = 0;
	m_output_reg = 0;
	m_stb = 0;
}

void t6b79_device::advance_y()
{
	if (m_active_counter)
	{
		m_ypos = (m_ypos + m_direction) & 0x0f;

		if (m_direction > 0 && m_ypos == (m_word_len ? 8 : 11))
			m_ypos = 0;
		if (m_direction < 0 && m_ypos == 0x0f)
			m_ypos = m_word_len ? 7 : 10;
	}
	else
	{
		m_xpos = (m_xpos + m_direction) & 0x3f;

		if (m_direction > 0 && m_xpos == 48)
			m_xpos = 0;
		if (m_direction < 0 && m_xpos == 0x3f)
			m_xpos = 47;
	}
}

uint32_t t6b79_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_display_on && !m_stb) {
        for (int x = 0; x < m_height; x++) {
            uint8_t src_x = (x + m_zpos) % m_height;
            for (int y = 0; y < (m_width >> 3); y++) {
                uint8_t data = m_lcd_ram[src_x*8 + y];
                for (int b = 0; b < 8; b++)
                    bitmap.pix(x, y*8 + b) = BIT(data, 7 - b);
            }
        }
	} else {
		bitmap.fill(0, cliprect);
	}

	return 0;
}

void t6b79_device::control_write(uint8_t data)
{
    
	if ((data & 0xc0) == 0xc0) // SCE (set contrast)
	{
		m_contrast = data & 0x3f;
	}
	else if ((data & 0xc0) == 0x80) // SXE (set x address)
	{
		m_xpos = data & 0x3f;
	}
	else if ((data & 0xc0) == 0x40) // SZE (set z address)
	{
		m_zpos = data & 0x3f;
	}
	else if ((data & 0xe0) == 0x20) // SYE (set y address)
	{
		m_ypos = data & 0x0f;
	}
	else if ((data & 0xf8) == 0x18) // CHE (test mode)
	{
		//???
	}
	else if ((data & 0xf8) == 0x10) // OPA2 (op-amp control 2)
	{
		m_opa2 = data & 3;
	}
	else if ((data & 0xf8) == 0x08) // OPA1 (op-amp control 1)
	{
		m_opa1 = data & 3;
	}
	else if ((data & 0xfc) == 0x04) // UDE (up/down mode)
	{
		m_active_counter = (data & 0x02) >> 1;
		m_direction = (data & 0x01) ? 1 : -1;
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

uint8_t t6b79_device::control_read()
{
	return (m_stb << 7) | (m_word_len << 6) | (m_display_on << 5) | (m_active_counter << 1) | (m_direction == 1 ? 1 : 0);
}

void t6b79_device::data_write(uint8_t data)
{
	if (m_xpos > 47 || m_ypos > 10 || (m_word_len && m_ypos > 7)) {
	} else if (m_word_len) {
        m_lcd_ram[m_xpos * 8 + m_ypos] = data;
	} else {
		uint8_t &slot = m_lcd_ram[m_xpos*8 + m_ypos];
        
        if (m_ypos != 10)
            slot = (slot & 0xc0) | (data & 0x3f);
        else
            slot = (slot & 0xc3) | (data & 0x3c);
	}

	advance_y();
}

uint8_t t6b79_device::data_read()
{
	uint8_t x = m_output_reg;

	if (m_stb)
		return 0x00;

	if (!machine().side_effects_disabled())
	{
		if (!m_word_len && m_ypos > 10)
			m_output_reg = 0x20 | m_ypos;
		else if (m_xpos > 47 || (m_word_len && m_ypos > 7))
			m_output_reg = m_word_len ? 0xff : 0x3c;
		else
		{
			int lo = (!m_word_len && m_ypos == 10) ? 2 : 0;
			int bits = m_word_len ? 8 : 6;
			uint8_t out = 0;
			for (int i = lo; i < bits; i++)
				out |= ((m_lcd_ram[m_xpos*8 + m_ypos] >> i) & 1) << i;
			m_output_reg = out;
		}

		advance_y();
	}

	return x;
}

void t6b79_device::stb_write(uint8_t data)
{
	m_stb = data & 1;
}

uint8_t t6b79_device::stb_read()
{
	return m_stb;
}