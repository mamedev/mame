// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_ops.h"

va_const_device::va_const_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VA_CONST, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_value(0)
{
}

va_const_device &va_const_device::set_value(float value)
{
	if (m_stream)
		m_stream->update();
	m_value = value;
	return *this;
}

void va_const_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_value));
}

void va_const_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, m_value);
}


va_scale_offset_device::va_scale_offset_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VA_SCALE_OFFSET, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_scale(1)
	, m_offset(0)
{
}

va_scale_offset_device &va_scale_offset_device::set_scale(float scale)
{
	if (m_stream)
		m_stream->update();
	m_scale = scale;
	return *this;
}

va_scale_offset_device &va_scale_offset_device::set_offset(float offset)
{
	if (m_stream)
		m_stream->update();
	m_offset = offset;
	return *this;
}

void va_scale_offset_device::device_start()
{
	save_item(NAME(m_scale));
	save_item(NAME(m_offset));
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
}

void va_scale_offset_device::sound_stream_update(sound_stream &stream)
{
	const int n = stream.samples();
	for (int i = 0; i < n; ++i)
		stream.put(0, i, m_scale * stream.get(0, i) + m_offset);
}


DEFINE_DEVICE_TYPE(VA_CONST, va_const_device, "va_const", "Constant value stream")
DEFINE_DEVICE_TYPE(VA_SCALE_OFFSET, va_scale_offset_device, "va_scale_offset", "Scale and offset")
