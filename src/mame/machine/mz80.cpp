// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        MZ80 driver by Miodrag Milanovic

        22/11/2008 Preliminary driver.

****************************************************************************/

#include "includes/mz80.h"


/* Driver initialization */
void mz80_state::init_mz80k()
{
}

void mz80_state::machine_reset()
{
	m_mz80k_tempo_strobe = 0;
	m_mz80k_vertical = 0;
	m_mz80k_cursor_cnt = 0;
	m_mz80k_keyboard_line = 0;
}


uint8_t mz80_state::mz80k_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	char kbdrow[8];
	sprintf(kbdrow,"LINE%d", m_mz80k_keyboard_line);
	if (m_mz80k_keyboard_line > 9)
		return 0xff;
	else
		return ioport(kbdrow)->read();
}

uint8_t mz80_state::mz80k_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t val = 0;
	val |= m_mz80k_vertical ? 0x80 : 0x00;
	val |= ((m_mz80k_cursor_cnt & 0x3f) > 31) ? 0x40 : 0x00;
	val |= (m_cassette->get_state() & CASSETTE_MASK_UISTATE)== CASSETTE_PLAY ? 0x10 : 0x00;

	if (m_cassette->input() > 0.00)
		val |= 0x20;

	return val;
}

void mz80_state::mz80k_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_mz80k_keyboard_line = data & 0x0f;
}

void mz80_state::mz80k_8255_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
//  logerror("mz80k_8255_portc_w %02x\n",data);
}


void mz80_state::pit_out0_changed(int state)
{
	if(!m_prev_state && state)
		m_speaker_level++;

	m_prev_state = state;
	m_speaker->level_w(BIT(m_speaker_level, 1));
}

void mz80_state::pit_out2_changed(int state)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

uint8_t mz80_state::mz80k_strobe_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0x7e | (uint8_t)m_mz80k_tempo_strobe;
}

void mz80_state::mz80k_strobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pit->write_gate0(BIT(data, 0));
}
