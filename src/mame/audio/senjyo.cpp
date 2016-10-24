// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#include "emu.h"
#include "includes/senjyo.h"


const z80_daisy_config senjyo_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ nullptr }
};


/* z80 pio */

uint8_t senjyo_state::pio_pa_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_sound_cmd;
}

void senjyo_state::sound_line_clock(int state)
{
	if (state != 0)
	{
		m_dac->write((m_sound_state & 8) ? m_single_volume : 0);
		m_sound_state++;
	}
}

void senjyo_state::volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_single_volume = data & 0x0f;
}
