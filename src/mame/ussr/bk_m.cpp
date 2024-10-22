// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
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

// SEL1 register (0010 and 0010.01)
//
// 15-8	R	high byte of cpu start address
// 7	R	bitbanger cts in
// 7	W	cassette motor control, 1: off 0: on
// 6	R	keyboard any key down, 1: no 0: yes
// 6	W	cassette data and speaker out
// 5	R	cassette data in
// 5	W	cassette data and bitbanger rts out
// 4	R	bitbanger rx
// 4	W	bitbanger tx
// 2	R	updated
//
// only original 0010 has bitbanger wired to UP connector

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
