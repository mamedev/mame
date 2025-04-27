// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_eg.h"
#include "machine/rescap.h"

// The envelope is considered completed after this many time constants.
static constexpr const float TIME_CONSTANTS_TO_END = 10;

DEFINE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device, "va_rc_eg", "RC-based Envelope Generator")

va_rc_eg_device::va_rc_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VA_RC_EG, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	// Initialize to a valid state.
	, m_r(RES_M(1))
	, m_c(CAP_U(1))
	, m_rc_inv(1.0F / (m_r * m_c))
	, m_v_start(0)
	, m_v_end(0)
{
}

va_rc_eg_device &va_rc_eg_device::set_r(float r)
{
	assert(r > 0);
	if (r == m_r)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();  // Snapshots voltage using the old `r` value.
	m_r = r;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_c(float c)
{
	assert(c > 0);
	if (c == m_c)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();  // Snapshots voltage using the old `c` value.
	m_c = c;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_target_v(float v)
{
	if (v == m_v_end)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();
	m_v_end = v;
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_instant_v(float v)
{
	if (m_stream != nullptr)
		m_stream->update();

	m_v_start = v;
	m_v_end = v;
	m_t_start = has_running_machine() ? machine().time() : attotime::zero;
	m_t_end_approx = m_t_start;
	return *this;
}

float va_rc_eg_device::get_v(const attotime &t) const
{
	assert(t >= m_t_start);
	const float delta_t = float((t - m_t_start).as_double());
	return m_v_start + (m_v_end - m_v_start) * (1.0F - expf(-delta_t * m_rc_inv));
}

float va_rc_eg_device::get_v() const
{
	return get_v(machine().time());
}

void va_rc_eg_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_r));
	save_item(NAME(m_c));
	save_item(NAME(m_rc_inv));
	save_item(NAME(m_v_start));
	save_item(NAME(m_v_end));
	save_item(NAME(m_t_start));
	save_item(NAME(m_t_end_approx));
}

void va_rc_eg_device::sound_stream_update(sound_stream &stream)
{
	assert(inputs.size() == 0 && outputs.size() == 1);
	attotime t = stream.start_time();

	if (t >= m_t_end_approx)
	{
		// Avoid expensive get_v() calls if the envelope stage has completed.
		stream.fill(0, m_v_end);
		return;
	}

	const int n = stream.samples();
	const attotime dt = stream.sample_period();
	for (int i = 0; i < n; ++i, t += dt)
		stream.put(0, i, get_v(t));
}

void va_rc_eg_device::snapshot()
{
	if (has_running_machine())
	{
		const attotime now = machine().time();
		m_v_start = get_v(now);
		m_t_start = now;
	}
	else
	{
		m_v_start = 0;
		m_t_start = attotime::zero;
	}
	m_t_end_approx = m_t_start + attotime::from_double(TIME_CONSTANTS_TO_END * m_r * m_c);
}
