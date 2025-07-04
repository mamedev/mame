// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  audio/special.cpp

  Functions to emulate sound hardware of Specialist MX
  ( based on code of DAI interface )

****************************************************************************/

#include "emu.h"
#include "specialsound.h"


// device type definition
DEFINE_DEVICE_TYPE(SPECIMX_SND, specimx_sound_device, "specimx_sound", "Specialist MX Custom Sound")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  specimx_sound_device - constructor
//-------------------------------------------------

specimx_sound_device::specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECIMX_SND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_mixer_channel(nullptr)
	, m_specimx_input{ 0, 0, 0 }
{
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

void specimx_sound_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t channel_0_signal = m_specimx_input[0] ? 0.1 : -0.1;
	sound_stream::sample_t channel_1_signal = m_specimx_input[1] ? 0.1 : -0.1;
	sound_stream::sample_t channel_2_signal = m_specimx_input[2] ? 0.1 : -0.1;
	sound_stream::sample_t sum = channel_0_signal + channel_1_signal + channel_2_signal;

	stream.fill(0, sum);
}


//-------------------------------------------------
//  PIT callbacks
//-------------------------------------------------

void specimx_sound_device::set_input_ch0(int state)
{
	m_mixer_channel->update();
	m_specimx_input[0] = state;
}

void specimx_sound_device::set_input_ch1(int state)
{
	m_mixer_channel->update();
	m_specimx_input[1] = state;
}

void specimx_sound_device::set_input_ch2(int state)
{
	m_mixer_channel->update();
	m_specimx_input[2] = state;
}
