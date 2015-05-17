// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/aztarac.h"


READ16_MEMBER(aztarac_state::sound_r)
{
	return m_sound_status & 0x01;
}

WRITE16_MEMBER(aztarac_state::sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		soundlatch_byte_w(space, offset, data);
		m_sound_status ^= 0x21;
		if (m_sound_status & 0x20)
			m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

READ8_MEMBER(aztarac_state::snd_command_r)
{
	m_sound_status |= 0x01;
	m_sound_status &= ~0x20;
	return soundlatch_byte_r(space,offset);
}

READ8_MEMBER(aztarac_state::snd_status_r)
{
	return m_sound_status & ~0x01;
}

WRITE8_MEMBER(aztarac_state::snd_status_w)
{
	m_sound_status &= ~0x10;
}

INTERRUPT_GEN_MEMBER(aztarac_state::snd_timed_irq)
{
	m_sound_status ^= 0x10;

	if (m_sound_status & 0x10)
		device.execute().set_input_line(0,HOLD_LINE);
}
