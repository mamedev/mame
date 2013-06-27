/***************************************************************************
    meadows.c
    Sound handler
    Dead Eye, Gypsy Juggler

    J. Buchmueller, June '98
****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "includes/meadows.h"
#include "sound/samples.h"
#include "sound/dac.h"



#define BASE_CLOCK      5000000
#define BASE_CTR1       (BASE_CLOCK / 256)
#define BASE_CTR2       (BASE_CLOCK / 32)

#define DIV2OR4_CTR2    0x01
#define ENABLE_CTR2     0x02
#define ENABLE_DAC      0x04
#define ENABLE_CTR1     0x08

static const INT16 waveform[2] = { -120*256, 120*256 };

/************************************/
/* Sound handler start              */
/************************************/
SAMPLES_START( meadows_sh_start )
{
	meadows_state *state = device.machine().driver_data<meadows_state>();
	state->m_0c00 = state->m_0c01 = state->m_0c02 = state->m_0c03 = 0;
	state->m_dac_data = 0;
	state->m_dac_enable = 0;
	state->m_channel = 0;
	state->m_freq1 = state->m_freq2 = 1000;
	state->m_latched_0c01 = state->m_latched_0c02 = state->m_latched_0c03 = 0;

	device.set_volume(0,0);
	device.start_raw(0,waveform,ARRAY_LENGTH(waveform),state->m_freq1,true);
	device.set_volume(1,0);
	device.start_raw(1,waveform,ARRAY_LENGTH(waveform),state->m_freq2,true);
}

/************************************/
/* Sound handler update             */
/************************************/
void meadows_sh_update(running_machine &machine)
{
	meadows_state *state = machine.driver_data<meadows_state>();
	int preset, amp;

	if (state->m_latched_0c01 != state->m_0c01 || state->m_latched_0c03 != state->m_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01 */
		/* and bit 4 merged from S2650's flag output */
		amp = ((state->m_0c03 & ENABLE_CTR1) == 0) ? 0 : (state->m_0c01 & 0xf0) >> 1;
		if( state->m_maincpu->state_int(S2650_FO) )
			amp += 0x80;
		/* calculate frequency for counter #1 */
		/* bit 0..3 of 0c01 are ctr preset */
		preset = (state->m_0c01 & 15) ^ 15;
		if (preset)
			state->m_freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		logerror("meadows ctr1 channel #%d preset:%3d freq:%5d amp:%d\n", state->m_channel, preset, state->m_freq1, amp);
		state->m_samples->set_frequency(0, state->m_freq1 * sizeof(waveform)/2);
		state->m_samples->set_volume(0,amp/255.0);
	}

	if (state->m_latched_0c02 != state->m_0c02 || state->m_latched_0c03 != state->m_0c03)
	{
		/* calculate frequency for counter #2 */
		/* 0c02 is ctr preset, 0c03 bit 0 enables division by 2 */
		amp = ((state->m_0c03 & ENABLE_CTR2) != 0) ? 0xa0 : 0;
		preset = state->m_0c02 ^ 0xff;
		if (preset)
		{
			state->m_freq2 = BASE_CTR2 / (preset + 1) / 2;
			if ((state->m_0c03 & DIV2OR4_CTR2) == 0)
				state->m_freq2 >>= 1;
		}
		else amp = 0;
		logerror("meadows ctr2 channel #%d preset:%3d freq:%5d amp:%d\n", state->m_channel+1, preset, state->m_freq2, amp);
		state->m_samples->set_frequency(1, state->m_freq2 * sizeof(waveform));
		state->m_samples->set_volume(1,amp/255.0);
	}

	if (state->m_latched_0c03 != state->m_0c03)
	{
		state->m_dac_enable = state->m_0c03 & ENABLE_DAC;

		if (state->m_dac_enable)
			state->m_dac->write_unsigned8(state->m_dac_data);
		else
			state->m_dac->write_unsigned8(0);
	}

	state->m_latched_0c01 = state->m_0c01;
	state->m_latched_0c02 = state->m_0c02;
	state->m_latched_0c03 = state->m_0c03;
}

/************************************/
/* Write DAC value                  */
/************************************/
void meadows_sh_dac_w(running_machine &machine, int data)
{
	meadows_state *state = machine.driver_data<meadows_state>();
	state->m_dac_data = data;
	if (state->m_dac_enable)
		state->m_dac->write_unsigned8(state->m_dac_data);
	else
		state->m_dac->write_unsigned8(0);
}
