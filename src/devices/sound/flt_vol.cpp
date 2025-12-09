// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
#include "emu.h"
#include "flt_vol.h"


// device type definition
DEFINE_DEVICE_TYPE(FILTER_VOLUME, filter_volume_device, "filter_volume", "Volume Filter")

//-------------------------------------------------
//  filter_volume_device - constructor
//-------------------------------------------------

filter_volume_device::filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FILTER_VOLUME, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_gain(1.0f)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_volume_device::device_start()
{
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_gain));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_volume_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
		stream.put(0, i, stream.get(0, i) * m_gain);
}


filter_volume_device &filter_volume_device::set_gain(float gain)
{
	if (m_stream)
		m_stream->update();
	m_gain = gain;

	return *this;
}
