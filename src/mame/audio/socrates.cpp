// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

    audio/socrates.c
    Copyright (C) 2010-2011 Jonathan Gevaryahu AKA Lord Nightmare

    This handles the two squarewaves (plus the one weird wave) channels
    on the V-tech Socrates system 27-0769 ASIC.


****************************************************************************/

#include "emu.h"
#include "socrates.h"


// device type definition
const device_type SOCRATES_SOUND = &device_creator<socrates_snd_device>;


//-------------------------------------------------
//  socrates_snd_device - constructor
//-------------------------------------------------

socrates_snd_device::socrates_snd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SOCRATES_SOUND, "Socrates Sound", tag, owner, clock, "socrates_snd", __FILE__),
	device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void socrates_snd_device::device_start()
{
	m_freq[0] = m_freq[1] = 0xff; /* channel 1,2 frequency */
	m_vol[0] = m_vol[1] = 0x07; /* channel 1,2 volume */
	m_enable[0] = m_enable[1] = 0x01; /* channel 1,2 enable */
	m_channel3 = 0x00; /* channel 3 weird register */
	m_DAC_output = 0x00; /* output */
	m_state[0] = m_state[1] = m_state[2] = 0;
	m_accum[0] = m_accum[1] = m_accum[2] = 0xFF;
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() ? clock() : machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void socrates_snd_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		snd_clock();
		outputs[0][i] = ((int)m_DAC_output<<4);
	}
}


const UINT8 socrates_snd_device::s_volumeLUT[16] =
{
0, 61, 100, 132, 158, 183, 201, 218,
233, 242, 253, 255, 250, 240, 224, 211
}; // this table is actually quite weird on the real console.
// 0, 0.033, 0.055, 0.07175, 0.086, 0.1, 0.11, 0.119, 0.127, 0.132, 0.138, 0.139, 0.136, 0.131, 0.122, 0.115 are the voltage amplitudes for the steps on channel 2. the last four are particularly bizarre, probably caused by some sort of internal clipping.

void socrates_snd_device::snd_clock() /* called once per clock */
{
	for (int channel = 0; channel < 2; channel++)
	{
		if ((m_accum[channel] == 0) && m_enable[channel])
		{
			m_state[channel] = (m_state[channel]^0x1);
			m_accum[channel] = m_freq[channel];
		}
		else if (m_enable[channel])
		{
			m_accum[channel]--;
		}
		else
		{
			m_accum[channel] = 0; // channel is disabled
			m_state[channel] = 0;
		}
	}
	// handle channel 3 here
	m_DAC_output = (m_state[0]?(s_volumeLUT[m_vol[0]]*9.4):0); // channel 1 is ~2.4 times as loud as channel 2
	m_DAC_output += (m_state[1]?(s_volumeLUT[m_vol[1]]<<2):0);
	// add channel 3 to dac output here
}


void socrates_snd_device::reg0_w(int data)
{
	m_stream->update();
	m_freq[0] = data;
}

void socrates_snd_device::reg1_w(int data)
{
	m_stream->update();
	m_freq[1] = data;
}

void socrates_snd_device::reg2_w(int data)
{
	m_stream->update();
	m_vol[0] = data&0xF;
	m_enable[0] = (data&0x10)>>4;
}

void socrates_snd_device::reg3_w(int data)
{
	m_stream->update();
	m_vol[1] = data&0xF;
	m_enable[1] = (data&0x10)>>4;
}

void socrates_snd_device::reg4_w(int data)
{
	m_stream->update();
	m_channel3 = data;
}
