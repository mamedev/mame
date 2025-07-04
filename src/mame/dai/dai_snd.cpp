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
DEFINE_DEVICE_TYPE(DAI_SOUND, dai_sound_device, "dai_sound", "DAI Custom Sound")


//-------------------------------------------------
//  dai_sound_device - constructor
//-------------------------------------------------

dai_sound_device::dai_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DAI_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dai_sound_device::device_start()
{
	m_mixer_channel = stream_alloc(0, 2, machine().sample_rate());
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

const uint16_t dai_sound_device::s_osc_volume_table[] = {
	   0,  500, 1000, 1500,
	2000, 2500, 3000, 3500,
	4000, 4500, 5000, 5500,
	6000, 6500, 7000, 7500
};

//-------------------------------------------------
//  noise volume table
//-------------------------------------------------

const uint16_t dai_sound_device::s_noise_volume_table[] = {
	   0,    0,    0,    0,
	   0,    0,    0,    0,
	 500, 1000, 1500, 2000,
	2500, 3000, 3500, 4000
};


//-------------------------------------------------
//  set_volume
//-------------------------------------------------

void dai_sound_device::set_volume(offs_t offset, uint8_t data)
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

void dai_sound_device::set_input_ch0(int state)
{
	m_mixer_channel->update();
	m_dai_input[0] = state;
}

void dai_sound_device::set_input_ch1(int state)
{
	m_mixer_channel->update();
	m_dai_input[1] = state;
}

void dai_sound_device::set_input_ch2(int state)
{
	m_mixer_channel->update();
	m_dai_input[2] = state;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void dai_sound_device::sound_stream_update(sound_stream &stream)
{
	int16_t channel_0_signal = m_dai_input[0] ? s_osc_volume_table[m_osc_volume[0]] : -s_osc_volume_table[m_osc_volume[0]];
	int16_t channel_1_signal = m_dai_input[1] ? s_osc_volume_table[m_osc_volume[1]] : -s_osc_volume_table[m_osc_volume[1]];
	int16_t channel_2_signal = m_dai_input[2] ? s_osc_volume_table[m_osc_volume[2]] : -s_osc_volume_table[m_osc_volume[2]];

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		int16_t noise = machine().rand()&0x01 ? s_noise_volume_table[m_noise_volume] : -s_noise_volume_table[m_noise_volume];

		/* channel 0 + channel 1 + noise */
		stream.put_int(0, sampindex, channel_0_signal + channel_1_signal + noise, 32768);

		/* channel 1 + channel 2 + noise */
		stream.put_int(1, sampindex, channel_1_signal + channel_2_signal + noise, 32768);
	}
}
