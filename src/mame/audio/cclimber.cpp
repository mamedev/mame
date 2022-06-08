// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "audio/cclimber.h"

#include "sound/ay8910.h"


// macro to convert 4-bit unsigned samples to 16-bit signed samples
#define SAMPLE_CONV4(a) (0x1111*((a&0x0f))-0x8000)

SAMPLES_START_CB_MEMBER( cclimber_audio_device::sh_start )
{
	m_sample_buf = std::make_unique<int16_t[]>(2 * m_samples_region.bytes());
	save_pointer(NAME(m_sample_buf), 2 * m_samples_region.bytes());
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CCLIMBER_AUDIO, cclimber_audio_device, "cclimber_audio", "Crazy Climber Sound Board")


//-------------------------------------------------
//  cclimber_audio_device: Constructor
//-------------------------------------------------

cclimber_audio_device::cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CCLIMBER_AUDIO, tag, owner, clock),
	m_sample_buf(nullptr),
	m_sample_num(0),
	m_sample_freq(0),
	m_sample_volume(0),
	m_sample_clockdiv(2),
	m_samples(*this, "samples"),
	m_samples_region(*this, "samples")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cclimber_audio_device::device_start()
{
	save_item(NAME(m_sample_num));
	save_item(NAME(m_sample_freq));
	save_item(NAME(m_sample_volume));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cclimber_audio_device::device_add_mconfig(machine_config &config)
{
	ay8910_device &aysnd(AY8910(config, "aysnd", DERIVED_CLOCK(1, 1)));
	aysnd.port_a_write_callback().set(FUNC(cclimber_audio_device::sample_select_w));
	aysnd.add_route(ALL_OUTPUTS, ":speaker", 0.5);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_start_callback(FUNC(cclimber_audio_device::sh_start));
	m_samples->add_route(ALL_OUTPUTS, ":speaker", 0.5);
}


void cclimber_audio_device::sample_select_w(uint8_t data)
{
	m_sample_num = data;
}

void cclimber_audio_device::sample_rate_w(uint8_t data)
{
	// calculate the sampling frequency
	m_sample_freq = clock() / m_sample_clockdiv / (256 - data);
}

void cclimber_audio_device::sample_volume_w(uint8_t data)
{
	m_sample_volume = data & 0x1f; // range 0-31
}

void cclimber_audio_device::sample_trigger(int state)
{
	if (state == 0)
		return;

	play_sample(32 * m_sample_num, m_sample_freq, m_sample_volume);
}

void cclimber_audio_device::sample_trigger_w(uint8_t data)
{
	sample_trigger(data != 0);
}


void cclimber_audio_device::play_sample(int start,int freq,int volume)
{
	int romlen = m_samples_region.bytes();

	// decode the ROM samples
	int len = 0;
	while (start + len < romlen && m_samples_region[start+len] != 0x70)
	{
		int sample;

		sample = (m_samples_region[start + len] & 0xf0) >> 4;
		m_sample_buf[2*len] = SAMPLE_CONV4(sample) * volume / 31;

		sample = m_samples_region[start + len] & 0x0f;
		m_sample_buf[2*len + 1] = SAMPLE_CONV4(sample) * volume / 31;

		len++;
	}

	m_samples->start_raw(0, m_sample_buf.get(), 2 * len, freq);
}
