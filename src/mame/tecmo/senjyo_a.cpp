// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#include "emu.h"
#include "senjyo.h"


const z80_daisy_config senjyo_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ nullptr }
};


/* z80 pio */

uint8_t senjyo_state::pio_pa_r()
{
	return m_sound_cmd;
}

WRITE_LINE_MEMBER(senjyo_state::sound_line_clock)
{
	if (state != 0)
	{
		m_dac->write((m_sound_state & 8) ? m_single_volume : 0);
		m_sound_state++;
	}
}

void senjyo_state::volume_w(uint8_t data)
{
	m_single_volume = data & 0x0f;
}
