// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    wave.c

    Code that interfaces
    Functions to handle loading, creation, recording and playback
    of wave samples for IO_CASSETTE

    2010-06-19 - Found that since 0.132, the right channel is badly out of
    sync on a mono system, causing bad sound. Added code to disable
    the second channel on a mono system.


****************************************************************************/

#include "emu.h"
#include "wave.h"

#include "speaker.h"


#define ALWAYS_PLAY_SOUND   0



DEFINE_DEVICE_TYPE(WAVE, wave_device, "wave", "Cassette Sound")

wave_device::wave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WAVE, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_cass(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wave_device::device_start()
{
	speaker_device_iterator spkiter(*owner());
	int speakers = spkiter.count();
	if (speakers > 1)
		machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	else
		machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wave_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	cassette_state state;
	double time_index;
	double duration;
	stream_sample_t *left_buffer = outputs[0];
	stream_sample_t *right_buffer = nullptr;
	int i;

	speaker_device_iterator spkiter(*owner());
	int speakers = spkiter.count();
	if (speakers>1)
		right_buffer = outputs[1];

	state = m_cass->get_state();

	state = (cassette_state)(state & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER));

	if (m_cass->exists() && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette_image *cassette = m_cass->get_image();
		time_index = m_cass->get_position();
		duration = ((double) samples) / m_cass->machine().sample_rate();

		cassette_get_samples(cassette, 0, time_index, duration, samples, 2, left_buffer, CASSETTE_WAVEFORM_16BIT);
		if (speakers > 1)
			cassette_get_samples(cassette, 1, time_index, duration, samples, 2, right_buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = samples - 1; i >= 0; i--)
		{
			left_buffer[i] = ((int16_t *) left_buffer)[i];
			if (speakers > 1)
				right_buffer[i] = ((int16_t *) right_buffer)[i];
		}
	}
	else
	{
		memset(left_buffer, 0, sizeof(*left_buffer) * samples);
		if (speakers > 1)
			memset(right_buffer, 0, sizeof(*right_buffer) * samples);
	}
}
