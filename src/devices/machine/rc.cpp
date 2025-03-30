// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "rc.h"
#include "machine/rescap.h"

DEFINE_DEVICE_TYPE(RC_CIRCUIT, rc_circuit_device, "rc_circuit", "DC RC circuit")

rc_circuit_device::rc_circuit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RC_CIRCUIT, tag, owner, clock)
	// Initialize to a valid state.
	, m_r(RES_M(1))
	, m_c(CAP_U(1))
	, m_rc_inv(1.0F / (m_r * m_c))
	, m_v_start(0)
	, m_v_end(0)
{
}

rc_circuit_device &rc_circuit_device::set_r(float r)
{
	assert(r > 0);
	set_target_v(m_v_end);  // Snapshots voltage, using the old `r` value.
	m_r = r;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

rc_circuit_device &rc_circuit_device::set_c(float c)
{
	assert(c > 0);
	set_target_v(m_v_end);  // Snapshots voltage, using the old `c` value.
	m_c = c;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

rc_circuit_device &rc_circuit_device::set_target_v(float v)
{
	if (has_running_machine())
	{
		const attotime now = machine().time();
		m_v_start = get_v(now);
		m_v_end = v;
		m_t_start = now;
	}
	else
	{
		m_v_start = 0;
		m_v_end = v;
		m_t_start = attotime::zero;
	}
	return *this;
}

rc_circuit_device &rc_circuit_device::set_instant_v(float v)
{
	m_v_start = v;
	m_v_end = v;
	m_t_start = has_running_machine() ? machine().time() : attotime::zero;
	return *this;
}

float rc_circuit_device::get_v(const attotime &t) const
{
	assert(t >= m_t_start);
	const float delta_t = float((t - m_t_start).as_double());
	return m_v_start + (m_v_end - m_v_start) * (1.0F - expf(-delta_t * m_rc_inv));
}

float rc_circuit_device::get_v() const
{
	return get_v(machine().time());
}

void rc_circuit_device::device_start()
{
	save_item(NAME(m_r));
	save_item(NAME(m_c));
	save_item(NAME(m_rc_inv));
	save_item(NAME(m_v_start));
	save_item(NAME(m_v_end));
	save_item(NAME(m_t_start));
}
