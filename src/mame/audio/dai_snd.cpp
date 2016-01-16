// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  audio/dai_snd.c

  Functions to emulate sound hardware of DAI Personal Computer

  Krzysztof Strzecha

****************************************************************************/

#include "emu.h"
#include "dai_snd.h"

// device type definition
const device_type DAI_SOUND = &device_creator<dai_sound_device>;


//-------------------------------------------------
//  dai_sound_device - constructor
//-------------------------------------------------

dai_sound_device::dai_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DAI_SOUND, "DAI Audio Custom", tag, owner, clock, "dai_sound", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dai_sound_device::device_start()
{
	m_mixer_channel = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dai_sound_device::device_reset()
{
	memset(m_dai_input, 0, sizeof(m_dai_input));
	memset(m_osc_volume, 0, sizeof(m_osc_volume));
	m_noise_volume = 0;
}


//-------------------------------------------------
//  channels 0/1/2 volume table
//-------------------------------------------------

const UINT16 dai_sound_device::s_osc_volume_table[] = {
						0,  500, 1000, 1500,
					2000, 2500, 3000, 3500,
					4000, 4500, 5000, 5500,
					6000, 6500, 7000, 7500
};

//-------------------------------------------------
//  noise volume table
//-------------------------------------------------

const UINT16 dai_sound_device::s_noise_volume_table[] = {
						0,    0,    0,    0,
						0,    0,    0,    0,
						500, 1000, 1500, 2000,
					2500, 3000, 3500, 4000
};


//-------------------------------------------------
//  set_volume
//-------------------------------------------------

WRITE8_MEMBER(dai_sound_device::set_volume)
{
	m_mixer_channel->update();

	switch (offset & 1)
	{
	case 0x00:
		m_osc_volume[0] = data&0x0f;
		m_osc_volume[1] = (data&0xf0)>>4;
		break;

	case 0x01:
		m_osc_volume[2] = data&0x0f;
		m_noise_volume = (data&0xf0)>>4;
	}
}

//-------------------------------------------------
//  PIT callbacks
//-------------------------------------------------

WRITE_LINE_MEMBER(dai_sound_device::set_input_ch0)
{
	m_mixer_channel->update();
	m_dai_input[0] = state;
}

WRITE_LINE_MEMBER(dai_sound_device::set_input_ch1)
{
	m_mixer_channel->update();
	m_dai_input[1] = state;
}

WRITE_LINE_MEMBER(dai_sound_device::set_input_ch2)
{
	m_mixer_channel->update();
	m_dai_input[2] = state;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void dai_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *sample_left = outputs[0];
	stream_sample_t *sample_right = outputs[1];

	INT16 channel_0_signal = m_dai_input[0] ? s_osc_volume_table[m_osc_volume[0]] : -s_osc_volume_table[m_osc_volume[0]];
	INT16 channel_1_signal = m_dai_input[1] ? s_osc_volume_table[m_osc_volume[1]] : -s_osc_volume_table[m_osc_volume[1]];
	INT16 channel_2_signal = m_dai_input[2] ? s_osc_volume_table[m_osc_volume[2]] : -s_osc_volume_table[m_osc_volume[2]];

	while (samples--)
	{
		INT16 noise = machine().rand()&0x01 ? s_noise_volume_table[m_noise_volume] : -s_noise_volume_table[m_noise_volume];

		/* channel 0 + channel 1 + noise */
		*sample_left++ = channel_0_signal + channel_1_signal + noise;

		/* channel 1 + channel 2 + noise */
		*sample_right++ = channel_1_signal + channel_2_signal + noise;
	}
}
