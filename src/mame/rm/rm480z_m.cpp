// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

/*

RM 480Z machine

*/

#include "emu.h"
#include "rm480z.h"

TIMER_DEVICE_CALLBACK_MEMBER(rm480z_state::kbd_scan)
{
	if (!m_kbd_reset && !m_kbd_ready)
	{
		int row = m_kbd_scan_pos >> 3;
		int col = m_kbd_scan_pos & 0x07;
		uint8_t new_val = m_io_kbrow[row]->read();
		uint8_t delta = new_val ^ m_kbd_state[row];
		uint8_t mask = 1 << col;

		if (delta & mask)
		{
			m_kbd_code = m_kbd_scan_pos;
			if ((new_val & mask) == 0)
			{
				m_kbd_code |= 0x80;
			}
			m_kbd_ready = true;
			m_kbd_state[row] ^= mask;
			m_ctc->trg2(0);
			m_ctc->trg2(1);
		}

		m_kbd_scan_pos++;
		m_kbd_scan_pos &= 0x3f;
	}
}

void rm480z_state::vblank_callback(screen_device &screen, bool vblank_state)
{
	if (vblank_state)
	{
		m_ctc->trg3(0);
		m_ctc->trg3(1);
	}
}

void rm480z_state::control_port_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		break;
	case 1:
		m_kbd_reset = !BIT(data, 6);
		m_view.select(data & 0x03);
		break;
	case 2:
		printf("control port write %d: %d\n", offset, data);
		m_speaker->level_w(BIT(data, 5));
		m_alt_char_set = BIT(data, 6);
		config_videomode(BIT(data, 7));
		break;
	case 3:
		break;
	case 5:
		break;	
	}
}

uint8_t rm480z_state::status_port_read(offs_t offset)
{
	uint8_t ret_val = 0;

	switch (offset)
	{
	case 0:
		break;
	case 1:
		// bit 0 is low during line blank
		if (!m_screen->hblank())
		{
			ret_val |= 0x01;
		}
		// bit 1 is high during frame blank
		if (m_screen->vblank())
		{
			ret_val |= 0x02;
		}
		// bit 3 is low when new key waiting to be read from kbd
		if (!m_kbd_ready)
		{
			ret_val |= 0x04;
		}
		break;
	case 2:
		break;
	case 3:
		ret_val = m_kbd_code;
		m_kbd_ready = false;
		break;
	case 5:
		break;	
	}	

	return ret_val;
}

uint8_t rm480z_state::hrg_port_read(offs_t offset)
{
	uint8_t ret_val = 0;

	switch (offset)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		// bit 0 is high during line blank
		if (m_screen->hblank())
		{
			ret_val |= 0x01;
		}
		// bit 1 is high during frame blank
		if (m_screen->vblank())
		{
			ret_val |= 0x02;
		}
		// bit 6 is low when screen memory is open
		if (!m_hrg_mem_open)
		{
			ret_val |= 0x40;
		}
		break;
	case 3:
		int index = calculate_hrg_vram_index();
		if (index >= 0)
		{
			ret_val = m_hrg_ram[index];
		}
		break;	
	}	

	return ret_val;
}

void rm480z_state::hrg_port_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_hrg_port0 = data;
		break;
	case 1:
		m_hrg_port1 = data;
		break;
	case 2:
		if (BIT(data, 7))
		{
			change_palette(data & 0x0f, 255 - m_hrg_port0);
		}
		else if (BIT(data, 1))
		{
			switch ((data >> 4) & 0x03)
			{
			case 0x00:
				m_hrg_display_mode = hrg_display_mode::extra_high;
				break;
			case 0x01:
				m_hrg_display_mode = hrg_display_mode::high;
				break;
			case 0x03:
				if (BIT(data, 3))
				{
					m_hrg_display_mode = hrg_display_mode::medium_1;
				}
				else
				{
					m_hrg_display_mode = hrg_display_mode::medium_0;
				}
				break;
			default:
				m_hrg_display_mode = hrg_display_mode::none;
				break;
			}
		}
		else
		{
			// HRG output inhibited when bit 1 is low
			m_hrg_display_mode = hrg_display_mode::none;
		}
		m_hrg_mem_open = !BIT(data, 6);
		break;
	case 3:
		int index = calculate_hrg_vram_index();
		if (index >= 0)
		{
			m_hrg_ram[index] = data;
		}
		break;	
	}
}

void rm480z_state::machine_reset()
{
	m_vram.reset();
	m_alt_char_set = false;

	m_kbd_reset = true;
	m_kbd_ready = false;
	m_kbd_code = 0;
	m_kbd_scan_pos = 0;
	memset(m_kbd_state, 0, sizeof(m_kbd_scan_pos));

	m_view.select(0);
}
