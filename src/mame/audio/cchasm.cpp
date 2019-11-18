// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Cinematronics Cosmic Chasm hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "includes/cchasm.h"
#include "sound/ay8910.h"


WRITE8_MEMBER(cchasm_state::reset_coin_flag_w)
{
	if (m_coin_flag)
	{
		m_coin_flag = 0;
		m_ctc->trg0(m_coin_flag);
	}
}

INPUT_CHANGED_MEMBER(cchasm_state::set_coin_flag )
{
	if (!newval && !m_coin_flag)
	{
		m_coin_flag = 1;
		m_ctc->trg0(m_coin_flag);
	}
}

READ8_MEMBER(cchasm_state::coin_sound_r)
{
	uint8_t coin = (ioport("IN3")->read() >> 4) & 0x7;
	return m_sound_flags | (m_coin_flag << 3) | coin;
}

READ8_MEMBER(cchasm_state::soundlatch2_r)
{
	m_sound_flags &= ~0x80;
	m_ctc->trg2(0);
	return m_soundlatch2->read();
}

WRITE8_MEMBER(cchasm_state::soundlatch4_w)
{
	m_sound_flags |= 0x40;
	m_soundlatch4->write(data);
	m_maincpu->set_input_line(1, HOLD_LINE);
}

WRITE16_MEMBER(cchasm_state::io_w)
{
	//static int led;

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (offset & 0xf)
		{
		case 0:
			m_soundlatch->write(data);
			break;
		case 1:
			m_sound_flags |= 0x80;
			m_soundlatch2->write(data);
			m_ctc->trg2(1);
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			break;
		case 2:
			//led = data;
			break;
		}
	}
}

READ16_MEMBER(cchasm_state::io_r)
{
	switch (offset & 0xf)
	{
	case 0x0:
		return m_soundlatch3->read() << 8;
	case 0x1:
		m_sound_flags &= ~0x40;
		return m_soundlatch4->read() << 8;
	case 0x2:
		return (m_sound_flags| (ioport("IN3")->read() & 0x07) | 0x08) << 8;
	case 0x5:
		return ioport("IN2")->read() << 8;
	case 0x8:
		return ioport("IN1")->read() << 8;
	default:
		return 0xff << 8;
	}
}


WRITE_LINE_MEMBER(cchasm_state::ctc_timer_1_w)
{
	if (state) /* rising edge */
	{
		m_output[0] = !m_output[0];
		m_dac1->write(m_output[0]);
	}
}

WRITE_LINE_MEMBER(cchasm_state::ctc_timer_2_w)
{
	if (state) /* rising edge */
	{
		m_output[1] = !m_output[1];
		m_dac2->write(m_output[1]);
	}
}

void cchasm_state::sound_start()
{
	m_coin_flag = 0;
	m_sound_flags = 0;
	m_output[0] = 0;
	m_output[1] = 0;

	save_item(NAME(m_sound_flags));
	save_item(NAME(m_coin_flag));
	save_item(NAME(m_output));
}
