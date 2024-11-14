// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    MP3 audio decoder

***************************************************************************/

#include "emu.h"
#include "mp3_audio.h"

#define MINIMP3_IMPLEMENTATION
#define MAX_FRAME_SYNC_MATCHES 2
#include "minimp3/minimp3.h"

// To avoid modifying minimp3.h, forward declare mp3dec_local_t in mp3_audio.h and then make it an mp3dec_t using inheritance
struct mp3_audio::mp3dec_local_t : public mp3dec_t
{
};

mp3_audio::mp3_audio(const void *_base)
	: base((const uint8_t *)_base)
{
	dec = std::make_unique<mp3dec_local_t>();
	clear();
}

mp3_audio::~mp3_audio()
{
}

void mp3_audio::register_save(device_t &host)
{
	host.save_item(NAME(m_found_stream));
	host.save_item(NAME(dec->header));
	host.save_item(NAME(dec->reserv_buf));
	host.save_item(NAME(dec->mdct_overlap));
	host.save_item(NAME(dec->qmf_state));
	host.save_item(NAME(dec->reserv));
	host.save_item(NAME(dec->free_format_bytes));
}

void mp3_audio::clear()
{
	mp3dec_init(dec.get());
	m_found_stream = false;
}

bool mp3_audio::decode_buffer(int &pos, int limit, short *output, int &output_samples, int &sample_rate, int &channels)
{
	mp3dec_frame_info_t info = {};

	if (!m_found_stream)
	{
		// Guarantee a specified number of frames are buffered before starting decoding to ensure it's not full of garbage that looks like a valid frame
		int free_format_bytes = 0;
		int frame_bytes = 0;
		int frame_offset = mp3d_find_frame(base, limit, &free_format_bytes, &frame_bytes);

		if (frame_bytes && frame_offset + frame_bytes <= limit)
		{
			int i = 0, nmatch = 0;

			for (i = frame_offset, nmatch = 0; nmatch < MAX_FRAME_SYNC_MATCHES; nmatch++)
			{
				i += hdr_frame_bytes(base + i, frame_bytes) + hdr_padding(base + i);
				if (i + HDR_SIZE > limit || !hdr_compare(base + frame_offset, base + i))
					break;
			}

			m_found_stream = nmatch >= MAX_FRAME_SYNC_MATCHES;
		}

		if (!m_found_stream)
		{
			output_samples = 0;
			sample_rate = 0;
			channels = 0;
			pos = 0;
			return false;
		}
	}

	output_samples = mp3dec_decode_frame(dec.get(), base, limit, output, &info);
	sample_rate = info.hz;
	channels = info.channels;
	pos = info.frame_bytes;

	return pos > 0 && output_samples > 0;
}
