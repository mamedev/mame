// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_EG_H
#define MAME_SOUND_VA_EG_H

#pragma once

DECLARE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device)
DECLARE_DEVICE_TYPE(CEM3310, cem3310_device)


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

	// These setters can be used during both: configuration and normal operation.
	// `r` and `c` must be > 0. To instantly set the capacitor's voltage to
	// a specific value, use set_instant_v().
	va_rc_eg_device &set_r(float r);
	va_rc_eg_device &set_c(float v);

	// Sets target voltage to (dis)charge towards.
	va_rc_eg_device &set_target_v(float v);
	float get_target_v() const { return m_v_end; }

	// Sets the voltage to the given value, instantly.
	va_rc_eg_device &set_instant_v(float v);

	float get_v(const attotime &t) const;
	float get_v() const;  // Get voltage at the machine's current time.

	// Returns the remaining time required to reach voltage `v`.
	// Returns attotime::never if `v` was reached in the past, or if it is
	// impossible to reach.
	attotime get_dt(float v) const;

	// Returns true if the voltage converged to the target by time `t`.
	bool converged(const attotime &t) const { return t >= m_t_end_approx; }

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	// Takes a snapshot of the current voltage into m_v_start and m_t_start.
	void snapshot();

	sound_stream *m_stream;

	float m_r;
	float m_c;
	float m_rc_inv;
	float m_v_start;
	float m_v_end;
	attotime m_t_start;
	attotime m_t_end_approx;
};


// Emulates a CEM3310 envelope generator.
// The rate CVs (attack, decay, release) are usually negative voltages.
// The sustain CV is a positive voltage, usually 0-5V.
// The output stream contains voltages, usually 0-5V.
//
// The emulation is accurate for the typical configuration. The behavior for
// less common configurations (e.g. sustain voltage above 5V, independent gate
// and trigger signals) is not yet emulated. The longer version of the datasheet
// has relevant info.
class cem3310_device : public device_t, public device_sound_interface
{
public:
	// rx - Resistor Rx connecting pin 10 (Iin) and pin 2 (ENV Out).
	// cx - Capacitor Cx at pin 1 (Cap).
	cem3310_device(const machine_config &mconfig, const char *tag, device_t *owner, float rx, float cx) ATTR_COLD;
	cem3310_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	void attack_w(float cv);
	void decay_w(float cv);
	void sustain_w(float cv);
	void release_w(float cv);

	// Assumes the trigger pin is connected to the gate pin via a capacitor.
	// Independent control of the trigger input is not yet emulated.
	void gate_w(int state);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void update_rc();
	TIMER_CALLBACK_MEMBER(attack_timer_tick);

	enum phase
	{
		PHASE_ATTACK = 0,
		PHASE_DECAY,
		PHASE_RELEASE
	};

	// Using the constant names in the datasheet.
	static inline constexpr float VP = 5.0F;  // Envelope peak voltage.
	static inline constexpr float VZ = 6.5F;  // Attack asymptote voltage (voltage target).
	static inline constexpr float VT = 0.026F;  // Thermal voltage at room temperature.

	const float m_rx;
	const float m_cx;

	sound_stream *m_stream;
	emu_timer *m_attack_timer;

	required_device<va_rc_eg_device> m_rc;

	s8 m_phase;
	bool m_gate;
	float m_attack_cv;
	float m_decay_cv;
	float m_sustain_cv;
	float m_release_cv;
};

#endif  // MAME_SOUND_VA_EG_H
