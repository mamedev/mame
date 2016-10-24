// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/aztarac.h"


uint16_t aztarac_state::sound_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_sound_status & 0x01;
}

void aztarac_state::sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		m_soundlatch->write(space, offset, data);
		m_sound_status ^= 0x21;
		if (m_sound_status & 0x20)
			m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

uint8_t aztarac_state::snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_sound_status |= 0x01;
	m_sound_status &= ~0x20;
	return m_soundlatch->read(space,offset);
}

uint8_t aztarac_state::snd_status_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_sound_status & ~0x01;
}

void aztarac_state::snd_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_sound_status &= ~0x10;
}

void aztarac_state::snd_timed_irq(device_t &device)
{
	m_sound_status ^= 0x10;

	if (m_sound_status & 0x10)
		device.execute().set_input_line(0,HOLD_LINE);
}
