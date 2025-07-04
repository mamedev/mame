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

	device_t *device = nullptr;
	sound_stream *mixer_channel = nullptr; // mame stream/ mixer channel

	int type = 0;
	uint32_t clock = 0;

	uint16_t PCMfreq = 0; // samplerate of the current systems soundcard/DAC
	uint32_t PCMsid = 0, PCMsidNoise = 0;

#if 0
	/* following depends on type */
	ptr2sidVoidFunc ModeNormalTable[16]{ nullptr };
	ptr2sidVoidFunc ModeRingTable[16]{ nullptr };
	// for speed reason it could be better to make them global!
	uint8_t *waveform30 = nullptr;
	uint8_t *waveform50 = nullptr;
	uint8_t *waveform60 = nullptr;
	uint8_t *waveform70 = nullptr;
#endif
	int reg[0x20]{ 0 };

//  bool sidKeysOn[0x20]{ false }, sidKeysOff[0x20]{ false };

	uint8_t masterVolume = 0;
	uint16_t masterVolumeAmplIndex = 0;

	struct
	{
		int Enabled = 0;
		uint8_t Type = 0, CurType = 0;
		float Dy = 0.0, ResDy = 0.0;
		uint16_t Value = 0;
	} filter;

	sidOperator optr[max_voices];
	int optr3_outputmask = 0;

	void init();

	bool reset();

	void postload();

	int port_r(running_machine &machine, int offset);
	void port_w(int offset, int data);

	void fill_buffer(sound_stream &stream);

private:
	void syncEm();
};

#endif // MAME_SOUND_SID_H
