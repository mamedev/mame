// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        BK machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "bk.h"


TIMER_CALLBACK_MEMBER(bk_state::keyboard_callback)
{
	uint8_t code, i, j;

	for(i = 1; i < 12; i++)
	{
		code = m_io_keyboard[i]->read();
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
			if (BIT(m_io_keyboard[0]->read(), 2))
			{
				if (i==6 || i==7)
					m_key_code -= 16;
				else
				if (i>=8 && i<=11)
					m_key_code += 32;
			}
			m_key_pressed = 0x40;

			if (!BIT(m_io_keyboard[0]->read(), 1))
				m_key_irq_vector = 0x30;
			else
				m_key_irq_vector = 0xBC;

			m_maincpu->set_input_line(0, ASSERT_LINE);
			break;
		}
	}
}


void bk_state::machine_start()
{
	save_item(NAME(m_scroll));
	save_item(NAME(m_kbd_state));
	save_item(NAME(m_key_code));
	save_item(NAME(m_key_pressed));
	save_item(NAME(m_key_irq_vector));
	save_item(NAME(m_drive));

	m_maincpu->set_input_line(t11_device::VEC_LINE, ASSERT_LINE);

	m_kbd_timer = timer_alloc(FUNC(bk_state::keyboard_callback), this);
	m_kbd_timer->adjust(attotime::from_hz(2400), 0, attotime::from_hz(2400));
}

uint8_t bk_state::irq_callback(offs_t offset)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return m_key_irq_vector;
}

void bk_state::machine_reset()
{
	m_kbd_state = 0;
	m_scroll = 01330;
}

uint16_t bk_state::key_state_r()
{
	return m_kbd_state;
}
uint16_t bk_state::key_code_r()
{
	m_kbd_state &= ~0x80; // mark reading done
	m_key_pressed = 0;
	return m_key_code;
}
uint16_t bk_state::vid_scroll_r()
{
	return m_scroll;
}

uint16_t bk_state::key_press_r()
{
	double level = m_cassette->input();
	uint16_t cas = (level < 0) ? 0 : 0x20;

	return 0x8080 | m_key_pressed | cas;
}

uint16_t bk_state::trap_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
	return ~0;
}

void bk_state::key_state_w(uint16_t data)
{
	m_kbd_state = (m_kbd_state & ~0x40) | (data & 0x40);
}

void bk_state::vid_scroll_w(uint16_t data)
{
	m_scroll = data;
}

void bk_state::key_press_w(uint16_t data)
{
	m_dac->write(BIT(data, 6));
	m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);
	m_cassette->change_state((BIT(data, 7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

void bk_state::trap_w(uint16_t data)
{
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
}

uint16_t bk_state::floppy_cmd_r()
{
	return 0;
}

void bk_state::floppy_cmd_w(uint16_t data)
{
	if (BIT(data, 0))
		m_drive = 0;

	if (BIT(data, 1))
		m_drive = 1;

	if (BIT(data, 2))
		m_drive = 2;

	if (BIT(data, 3))
		m_drive = 3;

	if (data == 0)
		m_drive = -1;
}

uint16_t bk_state::floppy_data_r()
{
	return 0;
}

void bk_state::floppy_data_w(uint16_t data)
{
}

u32 bk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 const mini = !BIT(m_scroll, 9);
	u16 const nOfs = (m_scroll & 255) + (mini ? 40 : -216);

	for (u16 y = 0; y < 256; y++)
	{
		for (u16 x = 0; x < 32; x++)
		{
			u16 const code = (y > 63 && mini) ? 0 : m_vram[((y+nOfs) %256)*32 + x];
			for (u8 b = 0; b < 16; b++)
				bitmap.pix(y, x*16 + b) =  BIT(code, b);
		}
	}
	return 0;
}
