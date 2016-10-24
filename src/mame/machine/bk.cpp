// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        BK machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "includes/bk.h"


void bk_state::keyboard_callback(void *ptr, int32_t param)
{
	uint8_t code, i, j;
	static const char *const keynames[] = {
		"LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6",
		"LINE7", "LINE8", "LINE9", "LINE10", "LINE11"
	};

	for(i = 1; i < 12; i++)
	{
		code =  ioport(keynames[i-1])->read();
		if (code != 0)
		{
			for(j = 0; j < 8; j++)
			{
				if (code == (1 << j))
				{
					m_key_code = j + i*8;
					break;
				}
			}
			if ((ioport("LINE0")->read() & 4) == 4)
			{
				if (i==6 || i==7)
				{
					m_key_code -= 16;
				}

			}
			if ((ioport("LINE0")->read() & 4) == 4)
			{
				if (i>=8 && i<=11)
				{
					m_key_code += 32;
				}
			}
			m_key_pressed = 0x40;
			if ((ioport("LINE0")->read() & 2) == 0)
			{
				m_key_irq_vector = 0x30;
			}
			else
			{
				m_key_irq_vector = 0xBC;
			}
			m_maincpu->set_input_line(0, ASSERT_LINE);
			break;
		}
	}
}


void bk_state::machine_start()
{
	machine().scheduler().timer_pulse(attotime::from_hz(2400), timer_expired_delegate(FUNC(bk_state::keyboard_callback),this));
}

int bk_state::bk0010_irq_callback(device_t &device, int irqline)
{
	device.execute().set_input_line(0, CLEAR_LINE);
	return m_key_irq_vector;
}

void bk_state::machine_reset()
{
	m_kbd_state = 0;
	m_scrool = 01330;
}

uint16_t bk_state::bk_key_state_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_kbd_state;
}
uint16_t bk_state::bk_key_code_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	m_kbd_state &= ~0x80; // mark reading done
	m_key_pressed = 0;
	return m_key_code;
}
uint16_t bk_state::bk_vid_scrool_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_scrool;
}

uint16_t bk_state::bk_key_press_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	double level = m_cassette->input();
	uint16_t cas;
	if (level < 0)
	{
		cas = 0x00;
	}
	else
	{
		cas = 0x20;
	}

	return 0x8080 | m_key_pressed | cas;
}

void bk_state::bk_key_state_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_kbd_state = (m_kbd_state & ~0x40) | (data & 0x40);
}

void bk_state::bk_vid_scrool_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_scrool = data;
}

void bk_state::bk_key_press_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
}

uint16_t bk_state::bk_floppy_cmd_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return 0;
}

void bk_state::bk_floppy_cmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((data & 1) == 1)
	{
		m_drive = 0;
	}
	if ((data & 2) == 2)
	{
		m_drive = 1;
	}
	if ((data & 4) == 4)
	{
		m_drive = 2;
	}
	if ((data & 8) == 8)
	{
		m_drive = 3;
	}
	if (data == 0)
	{
		m_drive = -1;
	}
}

uint16_t bk_state::bk_floppy_data_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return 0;
}

void bk_state::bk_floppy_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
}
