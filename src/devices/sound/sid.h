// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#ifndef MAME_SOUND_SID_H
#define MAME_SOUND_SID_H

#pragma once

/*
  approximation of the sid6581 chip
  this part is for one chip,
*/

#include "sidvoice.h"

/* private area */
struct SID6581_t
{
	static constexpr uint8_t max_voices = 3;

	device_t *device;
	sound_stream *mixer_channel; // mame stream/ mixer channel

	int type;
	uint32_t clock;

	uint16_t PCMfreq; // samplerate of the current systems soundcard/DAC
	uint32_t PCMsid, PCMsidNoise;

#if 0
	/* following depends on type */
	ptr2sidVoidFunc ModeNormalTable[16];
	ptr2sidVoidFunc ModeRingTable[16];
	// for speed reason it could be better to make them global!
	uint8_t* waveform30;
	uint8_t* waveform50;
	uint8_t* waveform60;
	uint8_t* waveform70;
#endif
	int reg[0x20];

//  bool sidKeysOn[0x20], sidKeysOff[0x20];

	uint8_t masterVolume;
	uint16_t masterVolumeAmplIndex;

	struct
	{
		int Enabled;
		uint8_t Type, CurType;
		float Dy, ResDy;
		uint16_t Value;
	} filter;

	sidOperator optr[max_voices];
	int optr3_outputmask;

	void init();

	bool reset();

	void postload();

	int port_r(running_machine &machine, int offset);
	void port_w(int offset, int data);

	void fill_buffer(stream_sample_t *buffer, uint32_t bufferLen);

private:
	void syncEm();
};

#endif // MAME_SOUND_SID_H
