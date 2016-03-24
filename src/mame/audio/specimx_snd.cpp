// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  audio/special.c

  Functions to emulate sound hardware of Specialist MX
  ( based on code of DAI interface )

****************************************************************************/

#include "specimx_snd.h"


// device type definition
const device_type SPECIMX_SND = &device_creator<specimx_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  specimx_sound_device - constructor
//-------------------------------------------------

specimx_sound_device::specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SPECIMX_SND, "Specialist MX Audio Custom", tag, owner, clock, "specimx_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_mixer_channel(nullptr)
{
	memset(m_specimx_input, 0, sizeof(int)*3);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void specimx_sound_device::device_start()
{
	m_specimx_input[0] = m_specimx_input[1] = m_specimx_input[2] = 0;
	m_mixer_channel = stream_alloc(0, 1, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void specimx_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	INT16 channel_0_signal;
	INT16 channel_1_signal;
	INT16 channel_2_signal;

	stream_sample_t *sample_left = outputs[0];

	channel_0_signal = m_specimx_input[0] ? 3000 : -3000;
	channel_1_signal = m_specimx_input[1] ? 3000 : -3000;
	channel_2_signal = m_specimx_input[2] ? 3000 : -3000;

	while (samples--)
	{
		*sample_left = 0;

		/* music channel 0 */
		*sample_left += channel_0_signal;

		/* music channel 1 */
		*sample_left += channel_1_signal;

		/* music channel 2 */
		*sample_left += channel_2_signal;

		sample_left++;
	}
}


//-------------------------------------------------
//  PIT callbacks
//-------------------------------------------------

WRITE_LINE_MEMBER(specimx_sound_device::set_input_ch0)
{
	m_mixer_channel->update();
	m_specimx_input[0] = state;
}

WRITE_LINE_MEMBER(specimx_sound_device::set_input_ch1)
{
	m_mixer_channel->update();
	m_specimx_input[1] = state;
}

WRITE_LINE_MEMBER(specimx_sound_device::set_input_ch2)
{
	m_mixer_channel->update();
	m_specimx_input[2] = state;
}
