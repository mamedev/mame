// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
    meadows.c
    Sound handler
    Dead Eye, Gypsy Juggler

    J. Buchmueller, June '98
****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "meadows.h"
#include "sound/samples.h"



#define BASE_CLOCK      5000000
#define BASE_CTR1       (BASE_CLOCK / 256)
#define BASE_CTR2       (BASE_CLOCK / 32)

#define DIV2OR4_CTR2    0x01
#define ENABLE_CTR2     0x02
#define ENABLE_DAC      0x04
#define ENABLE_CTR1     0x08

static const int16_t waveform[2] = { -120*256, 120*256 };

/************************************/
/* Sound handler start              */
/************************************/
SAMPLES_START_CB_MEMBER(meadows_state::meadows_sh_start)
{
	m_0c00 = m_0c01 = m_0c02 = m_0c03 = 0;
	m_channel = 0;
	m_freq1 = m_freq2 = 1000;
	m_latched_0c01 = m_latched_0c02 = m_latched_0c03 = 0;

	m_samples->set_volume(0,0);
	m_samples->start_raw(0,waveform,std::size(waveform),m_freq1,true);
	m_samples->set_volume(1,0);
	m_samples->start_raw(1,waveform,std::size(waveform),m_freq2,true);
}

/************************************/
/* Sound handler update             */
/************************************/
void meadows_state::meadows_sh_update()
{
	int preset, amp;

	if (m_latched_0c01 != m_0c01 || m_latched_0c03 != m_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01 */
		/* and bit 4 merged from S2650's flag output */
		amp = ((m_0c03 & ENABLE_CTR1) == 0) ? 0 : (m_0c01 & 0xf0) >> 1;
		if( m_maincpu->state_int(S2650_FO) )
			amp += 0x80;
		/* calculate frequency for counter #1 */
		/* bit 0..3 of 0c01 are ctr preset */
		preset = (m_0c01 & 15) ^ 15;
		if (preset)
			m_freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		logerror("meadows ctr1 channel #%d preset:%3d freq:%5d amp:%d\n", m_channel, preset, m_freq1, amp);
		m_samples->set_frequency(0, m_freq1 * sizeof(waveform)/2);
		m_samples->set_volume(0,amp/255.0);
	}

	if (m_latched_0c02 != m_0c02 || m_latched_0c03 != m_0c03)
	{
		/* calculate frequency for counter #2 */
		/* 0c02 is ctr preset, 0c03 bit 0 enables division by 2 */
		amp = ((m_0c03 & ENABLE_CTR2) != 0) ? 0xa0 : 0;
		preset = m_0c02 ^ 0xff;
		if (preset)
		{
			m_freq2 = BASE_CTR2 / (preset + 1) / 2;
			if ((m_0c03 & DIV2OR4_CTR2) == 0)
				m_freq2 >>= 1;
		}
		else amp = 0;
		logerror("meadows ctr2 channel #%d preset:%3d freq:%5d amp:%d\n", m_channel+1, preset, m_freq2, amp);
		m_samples->set_frequency(1, m_freq2 * sizeof(waveform));
		m_samples->set_volume(1,amp/255.0);
	}

	if (((m_latched_0c03 ^ m_0c03) & ENABLE_DAC) != 0)
	{
		m_dac->set_output_gain(ALL_OUTPUTS, (m_0c03 & ENABLE_DAC) != 0 ? 1.0 : 0.0);
	}

	m_latched_0c01 = m_0c01;
	m_latched_0c02 = m_0c02;
	m_latched_0c03 = m_0c03;
}
