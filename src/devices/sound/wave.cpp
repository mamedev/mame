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
	stream_alloc(0, 2, machine().sample_rate());
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wave_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	cassette_state state = m_cass->get_state() & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER);

	if (m_cass->exists() && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette_image *cassette = m_cass->get_image();
		double time_index = m_cass->get_position();
		double duration = double(outputs[0].samples()) / outputs[0].sample_rate();

		if (m_sample_buf.size() < outputs[0].samples())
			m_sample_buf.resize(outputs[0].samples());

		for (int ch = 0; ch < 2; ch++)
		{
			cassette->get_samples(ch, time_index, duration, outputs[ch].samples(), 2, &m_sample_buf[0], cassette_image::WAVEFORM_16BIT);
			for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
				outputs[ch].put_int(sampindex, m_sample_buf[sampindex], 32768);
		}
	}
	else
	{
		outputs[0].fill(0);
		outputs[1].fill(0);
	}
}
