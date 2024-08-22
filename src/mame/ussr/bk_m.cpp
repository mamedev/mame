// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        BK machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "bk.h"


void bk_state::machine_start()
{
	save_item(NAME(m_scroll));
	save_item(NAME(m_sel1));
	save_item(NAME(m_drive));
}

void bk_state::machine_reset()
{
	m_sel1 = SEL1_KEYDOWN | SEL1_MOTOR;
	m_scroll = 01330;
}

void bk_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_kbd->reset();
		m_qbus->init_w();
	}
}

uint16_t bk_state::vid_scroll_r()
{
	return m_scroll;
}

uint16_t bk_state::sel1_r()
{
	double level = m_cassette->input();
	uint16_t data = 0100000 | m_sel1 | ((level < 0) ? 0 : SEL1_RX_CAS);
	if (!machine().side_effects_disabled())
		m_sel1 &= ~SEL1_UPDATED;

	return data;
}

uint16_t bk_state::trap_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
	return ~0;
}

void bk_state::vid_scroll_w(uint16_t data)
{
	m_scroll = data & 01377;
}

void bk_state::sel1_w(uint16_t data)
{
	m_sel1 |= SEL1_UPDATED;
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
