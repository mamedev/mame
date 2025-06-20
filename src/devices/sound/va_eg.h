// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_EG_H
#define MAME_SOUND_VA_EG_H

#pragma once

// Building block for emulating envelope generators (EGs) based on a single RC
// circuit. The controlling source sets a target voltage and the device
// interpolates up or down to it through a RC circuit. The voltage is published
// as a sound stream and also available from `get_v()`.
//
// For example, emulating a CPU-controlled ADSR EG will look something like:
// - Machine configuration: rc_eg.set_c(C);
// - Start attack:          rc_eg.set_r(attack_r).set_target_v(max_v);
// - Start decay:           rc_eg.set_r(decay_r).set_target_v(sustain_v);
// - Start release:         rc_eg.set_r(release_r).set_target_v(0);
//
// The set_*() methods expect a monotonically increasing machine time, and will
// assert()-fail if that's not the case.
// If a single instance is used by multiple CPUs, this requirement can be met by
// calling set_*() from within timer callbacks invoked with:
// machine().scheduler().synchronize(timer_expired_delegate(...)).
// See src/mame/moog/source.cpp for examples.
//
// Depending on the details of the multi-CPU system being emulated, additional
// synchronization mitigations may be warranted, such as add_quantum(),
// set_perfect_quantum(), etc.
class va_rc_eg_device : public device_t, public device_sound_interface
{
public:
	va_rc_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// Skip creating a sound stream. Useful when using this device for RC
	// emulation other than sound.
	va_rc_eg_device &disable_streaming();

	// These setters can be used during both: configuration and normal operation.
	// `r` and `c` must be > 0. To instantly set the capacitor's voltage to
	// a specific value, use set_instant_v().
	va_rc_eg_device &set_r(float r);
	va_rc_eg_device &set_c(float v);

	// Sets target voltage to (dis)charge towards.
	va_rc_eg_device &set_target_v(float v);

	// Sets the voltage to the given value, instantly.
	va_rc_eg_device &set_instant_v(float v);

	float get_v(const attotime &t) const;
	float get_v() const;  // Get voltage at the machine's current time.

	// Returns the remaining time required to reach voltage `v`.
	// Returns attotime::never if `v` was reached in the past, or if it is
	// impossible to reach.
	attotime get_dt(float v) const;

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	// Takes a snapshot of the current voltage into m_v_start and m_t_start.
	void snapshot();

	sound_stream *m_stream;
	bool m_streaming;

	float m_r;
	float m_c;
	float m_rc_inv;
	float m_v_start;
	float m_v_end;
	attotime m_t_start;
	attotime m_t_end_approx;
};

DECLARE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device)

#endif  // MAME_SOUND_VA_EG_H
