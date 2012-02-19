/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  sound hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/polyplay.h"

#define LFO_VOLUME 25
#define SAMPLE_AMPLITUDE 0x4000



SAMPLES_START( polyplay_sh_start )
{
	polyplay_state *state = device.machine().driver_data<polyplay_state>();
	int i;

	for (i = 0; i < SAMPLE_LENGTH / 2; i++) {
		state->m_backgroundwave[i] = + SAMPLE_AMPLITUDE;
	}
	for (i = SAMPLE_LENGTH / 2; i < SAMPLE_LENGTH; i++) {
		state->m_backgroundwave[i] = - SAMPLE_AMPLITUDE;
	}
	state->m_freq1 = state->m_freq2 = 110;
	state->m_channel_playing1 = 0;
	state->m_channel_playing2 = 0;
}

void polyplay_set_channel1(running_machine &machine, int active)
{
	polyplay_state *state = machine.driver_data<polyplay_state>();
	state->m_channel_playing1 = active;
}

void polyplay_set_channel2(running_machine &machine, int active)
{
	polyplay_state *state = machine.driver_data<polyplay_state>();
	state->m_channel_playing2 = active;
}

void polyplay_play_channel1(running_machine &machine, int data)
{
	polyplay_state *state = machine.driver_data<polyplay_state>();
	samples_device *samples = machine.device<samples_device>("samples");
	if (data) {
		state->m_freq1 = 2457600 / 16 / data / 8;
		samples->set_volume(0, state->m_channel_playing1 * 1.0);
		samples->start_raw(0, state->m_backgroundwave, ARRAY_LENGTH(state->m_backgroundwave), sizeof(state->m_backgroundwave)*state->m_freq1,true);
	}
	else {
		samples->stop(0);
		samples->stop(1);
	}
}

void polyplay_play_channel2(running_machine &machine, int data)
{
	polyplay_state *state = machine.driver_data<polyplay_state>();
	samples_device *samples = machine.device<samples_device>("samples");
	if (data) {
		state->m_freq2 = 2457600 / 16 / data / 8;
		samples->set_volume(1, state->m_channel_playing2 * 1.0);
		samples->start_raw(1, state->m_backgroundwave, ARRAY_LENGTH(state->m_backgroundwave), sizeof(state->m_backgroundwave)*state->m_freq2,true);
	}
	else {
		samples->stop(0);
		samples->stop(1);
	}
}
