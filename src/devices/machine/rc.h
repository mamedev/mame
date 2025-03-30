// license:BSD-3-Clause
// copyright-holders:m1macrophage
#ifndef MAME_MACHINE_RC_H
#define MAME_MACHINE_RC_H

#pragma once

// Emulates a DC RC circuit with an (optionally) variable resistance and
// capacitance.
// Can be used to emulate the state of enveloper generators, timing circuits, etc.
// This is not a simulation. It just uses the standard RC equations to calculate
// voltage at a specific time.
// This device is not meant for audio stream processing. See FILTER_RC for that.
class rc_circuit_device : public device_t
{
public:
	rc_circuit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// These setters can be used during both: configuration and normal operation.
	// `r` and `c` must be > 0. To instantly set the capacitor's voltage to
	// a specific value, use set_instant_v().
	rc_circuit_device &set_r(float r);
	rc_circuit_device &set_c(float c);
	// Sets target voltage to (dis)charge towards.
	rc_circuit_device &set_target_v(float v);
	// Instantly sets voltage to the provided value.
	rc_circuit_device &set_instant_v(float v);

	float get_r() const { return m_r; }
	float get_c() const { return m_c; }
	float get_target_v() const { return m_v_end; }

	float get_v(const attotime &t) const;
	float get_v() const;

protected:
	void device_start() override ATTR_COLD;

private:
	float m_r;
	float m_c;
	float m_rc_inv;
	float m_v_start;
	float m_v_end;
	attotime m_t_start;
};

DECLARE_DEVICE_TYPE(RC_CIRCUIT, rc_circuit_device)

#endif // MAME_MACHINE_RC_H
