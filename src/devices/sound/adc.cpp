// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADCs, unsigned or signed two-complement

#include "emu.h"
#include "adc.h"


zn449_device::zn449_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZN449, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(nullptr),
	  m_current_value(0)
{
}

u8 zn449_device::read()
{
	m_stream->update();
	return m_current_value;
}

void zn449_device::device_start()
{
	save_item(NAME(m_current_value));
	m_stream = stream_alloc(1, 0, SAMPLE_RATE_INPUT_ADAPTIVE);
}

void zn449_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t last_sample = stream.get(0, stream.samples()-1);
	m_current_value = 128 * last_sample + 128;
}

DEFINE_DEVICE_TYPE(ZN449, zn449_device, "zn449", "ZN449 ADC")



adc10_device::adc10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADC10, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(nullptr),
	  m_current_value(0)
{
}

u16 adc10_device::read()
{
	m_stream->update();
	return m_current_value;
}

void adc10_device::device_start()
{
	save_item(NAME(m_current_value));
	m_stream = stream_alloc(1, 0, SAMPLE_RATE_INPUT_ADAPTIVE);
}

void adc10_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t last_sample = stream.get(0, stream.samples()-1);
	m_current_value = std::clamp(int(512 * last_sample), -512, 511);
}

DEFINE_DEVICE_TYPE(ADC10, adc10_device, "adc10", "10-bits signed ADC")
