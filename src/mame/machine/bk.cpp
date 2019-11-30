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
	m_kbd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bk_state::keyboard_callback), this));
	m_kbd_timer->adjust(attotime::from_hz(2400), 0, attotime::from_hz(2400));
}

IRQ_CALLBACK_MEMBER(bk_state::bk0010_irq_callback)
{
	device.execute().set_input_line(0, CLEAR_LINE);
	return m_key_irq_vector;
}

void bk_state::machine_reset()
{
	m_kbd_state = 0;
	m_scrool = 01330;
}

READ16_MEMBER(bk_state::bk_key_state_r)
{
	return m_kbd_state;
}
READ16_MEMBER(bk_state::bk_key_code_r)
{
	m_kbd_state &= ~0x80; // mark reading done
	m_key_pressed = 0;
	return m_key_code;
}
READ16_MEMBER(bk_state::bk_vid_scrool_r)
{
	return m_scrool;
}

READ16_MEMBER(bk_state::bk_key_press_r)
{
	double level = m_cassette->input();
	uint16_t cas = (level < 0) ? 0 : 0x20;

	return 0x8080 | m_key_pressed | cas;
}

WRITE16_MEMBER(bk_state::bk_key_state_w)
{
	m_kbd_state = (m_kbd_state & ~0x40) | (data & 0x40);
}

WRITE16_MEMBER(bk_state::bk_vid_scrool_w)
{
	m_scrool = data;
}

WRITE16_MEMBER(bk_state::bk_key_press_w)
{
	m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);
}

READ16_MEMBER(bk_state::bk_floppy_cmd_r)
{
	return 0;
}

WRITE16_MEMBER(bk_state::bk_floppy_cmd_w)
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

READ16_MEMBER(bk_state::bk_floppy_data_r)
{
	return 0;
}

WRITE16_MEMBER(bk_state::bk_floppy_data_w)
{
}
