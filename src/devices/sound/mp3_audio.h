// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    MP3 audio decoder

***************************************************************************/

#ifndef MAME_SOUND_MP3_AUDIO_H
#define MAME_SOUND_MP3_AUDIO_H

#pragma once

#include <stdint.h>

class mp3_audio
{
public:
	mp3_audio(const void *base);
	~mp3_audio();

	void register_save(device_t &host);

	bool decode_buffer(int &pos, int limit, short *output, int &output_samples, int &sample_rate, int &channels);

	void clear();

private:
	struct mp3dec_local_t;

	const uint8_t *base;

	std::unique_ptr<mp3dec_local_t> dec;
	bool m_found_stream;
};

#endif
