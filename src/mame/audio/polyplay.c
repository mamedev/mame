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
#define SAMPLE_LENGTH 32
#define SAMPLE_AMPLITUDE 0x4000

static int freq1, freq2, channel_playing1, channel_playing2;

static INT16 backgroundwave[SAMPLE_LENGTH];

SAMPLES_START( polyplay_sh_start )
{
	int i;

	for (i = 0; i < SAMPLE_LENGTH / 2; i++) {
		backgroundwave[i] = + SAMPLE_AMPLITUDE;
	}
	for (i = SAMPLE_LENGTH / 2; i < SAMPLE_LENGTH; i++) {
		backgroundwave[i] = - SAMPLE_AMPLITUDE;
	}
	freq1 = freq2 = 110;
	channel_playing1 = 0;
	channel_playing2 = 0;
}

void polyplay_set_channel1(int active)
{
	channel_playing1 = active;
}

void polyplay_set_channel2(int active)
{
	channel_playing2 = active;
}

void polyplay_play_channel1(running_machine *machine, int data)
{
	device_t *samples = machine->device("samples");
	if (data) {
		freq1 = 2457600 / 16 / data / 8;
		sample_set_volume(samples, 0, channel_playing1 * 1.0);
		sample_start_raw(samples, 0, backgroundwave, ARRAY_LENGTH(backgroundwave), sizeof(backgroundwave)*freq1,1);
	}
	else {
		sample_stop(samples, 0);
		sample_stop(samples, 1);
	}
}

void polyplay_play_channel2(running_machine *machine, int data)
{
	device_t *samples = machine->device("samples");
	if (data) {
		freq2 = 2457600 / 16 / data / 8;
		sample_set_volume(samples, 1, channel_playing2 * 1.0);
		sample_start_raw(samples, 1, backgroundwave, ARRAY_LENGTH(backgroundwave), sizeof(backgroundwave)*freq2,1);
	}
	else {
		sample_stop(samples, 0);
		sample_stop(samples, 1);
	}
}
