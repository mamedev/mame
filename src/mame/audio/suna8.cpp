// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*
    SunA 8 Bit Games samples

    Format: PCM unsigned 4 bit mono 8kHz
*/

#include "emu.h"
#include "sound/samples.h"
#include "includes/suna8.h"

#define FREQ_HZ 8000
#define SAMPLEN 0x1000

SAMPLES_START_CB_MEMBER(suna8_state::sh_start)
{
	int i, len = memregion("samples")->bytes() * 2;  // 2 samples per byte
	UINT8 *ROM = memregion("samples")->base();

	m_samplebuf = auto_alloc_array(machine(), INT16, len);

	// Convert 4 bit to 16 bit samples
	for(i = 0; i < len; i++)
		m_samplebuf[i] = (INT8)(((ROM[i/2] << ((i & 1)?0:4)) & 0xf0)  ^ 0x80) * 0x100;

	m_numsamples = len / SAMPLEN;
}

WRITE8_MEMBER(suna8_state::suna8_samples_number_w)
{
	m_sample = data;
	logerror("%s: sample number = %02X\n", machine().describe_context(), data);
}

void suna8_state::play_sample(int index)
{
	if (index < m_numsamples)
	{
		m_samples->start_raw(0, &m_samplebuf[SAMPLEN * index], SAMPLEN, FREQ_HZ);
		logerror("%s: starting sample %02X\n", machine().describe_context(), index);
	}
	else
	{
		logerror("%s: warning, invalid sample %02X\n", machine().describe_context(), index);
	}
}

WRITE8_MEMBER(suna8_state::suna8_play_samples_w)
{
	logerror("%s: play sample = %02X\n", machine().describe_context(), data);

	// At boot: ff (ay reset) -> 00 (game writes ay enable) -> f9 (game writes to port A).
	// Then game writes f9 -> f1 -> f9. Is bit 3 stop/reset?

	if ( m_play == 0xe9 && data == 0xf9 )
		play_sample(m_sample & 0x0f);
	else if ( m_play == 0xb9 && data == 0xf9 ) // second sample rom
		play_sample(((m_sample >> 4) & 0x0f) + 0x10);

	m_play = data;
}

WRITE8_MEMBER(suna8_state::rranger_play_samples_w)
{
	logerror("%s: play sample = %02X\n", machine().describe_context(), data);

	// At boot: ff (ay reset) -> 00 (game writes ay enable) -> 30 (game writes to port A).
	// Is bit 6 stop/reset?

	if ( m_play == 0x60 && data == 0x70 )
		play_sample(m_sample & 0x0f);

	m_play = data;
}
