// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    Cassette wave samples sound driver

    Code that interfaces functions to handle loading, creation,
    recording and playback of wave samples for IO_CASSETTE

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

void wave_device::sound_stream_update(sound_stream &stream)
{
	cassette_state state = m_cass->get_state() & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER);

	if (m_cass->exists() && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette_image *cassette = m_cass->get_image();
		double time_index = m_cass->get_position();
		double duration = double(stream.samples()) / stream.sample_rate();

		if (m_sample_buf.size() < stream.samples())
			m_sample_buf.resize(stream.samples());

		for (int ch = 0; ch < 2; ch++)
		{
			cassette->get_samples(ch, time_index, duration, stream.samples(), 2, &m_sample_buf[0], cassette_image::WAVEFORM_16BIT);
			for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
				stream.put_int(ch, sampindex, m_sample_buf[sampindex], 32768);
		}
	}
}
