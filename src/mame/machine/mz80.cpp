// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        MZ80 driver by Miodrag Milanovic

        22/11/2008 Preliminary driver.

****************************************************************************/

#include "includes/mz80.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(mz80_state,mz80k)
{
}

void mz80_state::machine_reset()
{
	m_mz80k_tempo_strobe = 0;
	m_mz80k_vertical = 0;
	m_mz80k_cursor_cnt = 0;
	m_mz80k_keyboard_line = 0;
}


READ8_MEMBER( mz80_state::mz80k_8255_portb_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"LINE%d", m_mz80k_keyboard_line);
	if (m_mz80k_keyboard_line > 9)
		return 0xff;
	else
		return ioport(kbdrow)->read();
}

READ8_MEMBER( mz80_state::mz80k_8255_portc_r )
{
	UINT8 val = 0;
	val |= m_mz80k_vertical ? 0x80 : 0x00;
	val |= ((m_mz80k_cursor_cnt & 0x3f) > 31) ? 0x40 : 0x00;
	val |= (m_cassette->get_state() & CASSETTE_MASK_UISTATE)== CASSETTE_PLAY ? 0x10 : 0x00;

	if (m_cassette->input() > 0.00)
		val |= 0x20;

	return val;
}

WRITE8_MEMBER( mz80_state::mz80k_8255_porta_w )
{
	m_mz80k_keyboard_line = data & 0x0f;
}

WRITE8_MEMBER( mz80_state::mz80k_8255_portc_w )
{
//  logerror("mz80k_8255_portc_w %02x\n",data);
}


WRITE_LINE_MEMBER( mz80_state::pit_out0_changed )
{
	if(!m_prev_state && state)
		m_speaker_level++;

	m_prev_state = state;
	m_speaker->level_w(BIT(m_speaker_level, 1));
}

WRITE_LINE_MEMBER( mz80_state::pit_out2_changed )
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

READ8_MEMBER( mz80_state::mz80k_strobe_r )
{
	return 0x7e | (UINT8)m_mz80k_tempo_strobe;
}

WRITE8_MEMBER( mz80_state::mz80k_strobe_w )
{
	m_pit->write_gate0(BIT(data, 0));
}
