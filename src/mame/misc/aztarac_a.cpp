// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "aztarac.h"


uint16_t aztarac_state::sound_r()
{
	return m_sound_status & 0x01;
}

void aztarac_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		m_soundlatch->write(data);
		m_sound_status ^= 0x21;
		if (m_sound_status & 0x20)
			m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

uint8_t aztarac_state::snd_command_r()
{
	m_sound_status |= 0x01;
	m_sound_status &= ~0x20;
	return m_soundlatch->read();
}

uint8_t aztarac_state::snd_status_r()
{
	return m_sound_status & ~0x01;
}

void aztarac_state::snd_status_w(uint8_t data)
{
	m_sound_status &= ~0x10;
}

INTERRUPT_GEN_MEMBER(aztarac_state::snd_timed_irq)
{
	m_sound_status ^= 0x10;

	if (m_sound_status & 0x10)
		device.execute().set_input_line(0,HOLD_LINE);
}
