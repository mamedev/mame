// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_ops.h"
#include "machine/rescap.h"


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


va_comparator_device::va_comparator_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VA_COMPARATOR, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_thresh_pos(0.5F)
	, m_thresh_neg(0.5F)
	, m_inverted(false)
	, m_stream(nullptr)
	, m_state(false)
{
}

va_comparator_device &va_comparator_device::configure(float thresh_pos, float thresh_neg, bool inverted)
{
	if (thresh_neg > thresh_pos)
		fatalerror("%s: negative-going threshold needs to be <= positive-going threshold\n", tag());
	m_thresh_pos = thresh_pos;
	m_thresh_neg = thresh_neg;
	m_inverted = inverted;
	return *this;
}

va_comparator_device &va_comparator_device::configure(const comp_oc_hyst_config &c)
{
	return configure(
		(c.v_thresh - c.v_pullup) * RES_VOLTAGE_DIVIDER(c.r_thresh, c.r_feedback + c.r_pullup) + c.v_pullup,
		(c.v_thresh - c.v_minus) * RES_VOLTAGE_DIVIDER(c.r_thresh, c.r_feedback) + c.v_minus, true);
}

int va_comparator_device::state()
{
	m_stream->update();
	if (m_inverted)
		return m_state ? 0 : 1;
	else
		return m_state ? 1 : 0;
}

void va_comparator_device::device_start()
{
	m_stream = stream_alloc(1, 0, SAMPLE_RATE_INPUT_ADAPTIVE);
	save_item(NAME(m_state));
}

void va_comparator_device::sound_stream_update(sound_stream &stream)
{
	const int n = stream.samples();
	for (int i = 0; i < n; ++i)
	{
		const float threshold = m_state ? m_thresh_neg : m_thresh_pos;
		m_state = stream.get(0, i) >= threshold;
	}
}


DEFINE_DEVICE_TYPE(VA_CONST, va_const_device, "va_const", "Constant value stream")
DEFINE_DEVICE_TYPE(VA_SCALE_OFFSET, va_scale_offset_device, "va_scale_offset", "Stream scale and offset")
DEFINE_DEVICE_TYPE(VA_COMPARATOR, va_comparator_device, "va_comparator", "Stream comparator")
