// license:???
// copyright-holders:Derrick Renaud, Couriersud
#include "emu.h"
#include "flt_vol.h"


// device type definition
const device_type FILTER_VOLUME = &device_creator<filter_volume_device>;

//-------------------------------------------------
//  filter_volume_device - constructor
//-------------------------------------------------

filter_volume_device::filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FILTER_VOLUME, "Volume Filter", tag, owner, clock, "filter_volume", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_gain(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_volume_device::device_start()
{
	m_gain = 0x100;
	m_stream = stream_alloc(1, 1, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_volume_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];

	while (samples--)
		*dst++ = (*src++ * m_gain) >> 8;
}



void filter_volume_device::flt_volume_set_volume(float volume)
{
	m_gain = (int)(volume * 256);
}
