// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    MP3 audio decoder

***************************************************************************/

#ifndef MAME_SOUND_MP3_AUDIO_H
#define MAME_SOUND_MP3_AUDIO_H

#pragma once

#include <stdint.h>

#include "minimp3/minimp3.h"

class mp3_audio
{
public:
	mp3_audio(const void *base);

	bool decode_buffer(int &pos, int limit, short *output, int &output_samples, int &sample_rate, int &channels);

	void clear();

private:
	const uint8_t *base;

	mp3dec_t dec;
	bool m_found_stream;
};

#endif
