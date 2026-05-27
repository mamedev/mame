// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_EG_H
#define MAME_SOUND_VA_EG_H

#pragma once

DECLARE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device)
DECLARE_DEVICE_TYPE(VA_OTA_EG, va_ota_eg_device)


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

// An envelope generator (EG) built around an operational transconductance
// amplifier (OTA). Its rate is controlled by Iabc.
//
// When the EG output is far from the target (e.g. > ~0.2V), the EG will ramp
// towards the target linearly. As the output approaches the target, the EG ramp
// will start curving. When the difference is < ~0.02V, the ramp  will closely
// approximate an "RC" curve with R = (2 * VT) / (s * Iabc). Where 's' is a
// device-specific constant (typically 1) and VT is the thermal voltage.
//
//                                x      Iabc
//                             ___|__     |
//                            |   |  \    |
//        Target V ---------- |+  Id  \   |
//                            |        \  |
//                            |         \ |
//                            |   OTA    OO ---+---- BUFFER --- EG output V
//                            |         /      |                |
//                            |        /       C                |
//                   +------- |-      /        |                |
//                   |        |______/        GND               |
//                   |                                          |
//                   +------------------------------------------+
//
// Note that this configuration does not make use of the linearizing diodes (Id
// input). The emulation here assumes the Id input is disconnected or connected
// to the negative supply, for OTAs that have it.
//
// A variant of the above uses anti-parallel diodes to connect the inputs (e.g.
// prophet 5 glide EG). This configuration protects the OTA from large voltage
// differences, but otherwise behaves the same to the above. The values of R
// don't matter.
//
//                                x      Iabc
//                             ___|__     |
//                            |   |  \    |
// Target V --- R ---+---+--- |+  Id  \   |
//                   |   |    |        \  |
//                   D   ^    |         \ |
//                   v   D    |   OTA    OO ---+---- BUFFER --- EG output V
//                   |   |    |         /      |                |
//                   |   |    |        /       C                |
//                   +---+--- |-      /        |                |
//                   |        |______/        GND               |
//                   |                                          |
//                   +-------------------- R -------------------+
//
// Another common variant uses voltage dividers at the two inputs. These ensure
// the voltage difference at the inputs is small, keeping the OTA in its linear
// region, and producing "RC" curves. In this configuration, the output will
// converge to: TargetV * scale+ / scale-, where scale = Rgnd / (Rin + Rgnd).
// For small enough scaling factors, R ~= (2 * VT) / (s * Iabc * scale-).
//
//                                x      Iabc
//                             ___|__     |
//                            |   |  \    |
// Target V - Rin+- -+------- |+  Id  \   |
//                   |        |        \  |
//                 Rgnd+      |         \ |
//                   |        |   OTA    OO ---+---- BUFFER --- EG output V
//                  GND       |         /      |                |
//                            |        /       C                |
//                   +------- |-      /        |                |
//                   |        |______/        GND               |
//                   |                                          |
// GND --- RGnd- --- +-------------------- Rin- ----------------+
//
class va_ota_eg_device : public device_t, public device_sound_interface
{
public:
	enum class ota_type : s8
	{
		LM13600 = 0,
		LM13700,
		CA3080,
		CA3280,
	};

	va_ota_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, ota_type ota, float c) ATTR_COLD;
	va_ota_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	// These configure the "scaled input" variant.
	va_ota_eg_device &configure_plus_divider(float r_in, float r_gnd);
	va_ota_eg_device &configure_minus_divider(float r_in, float r_gnd);

	void set_target_v(float v);
	void set_iabc(float iabc);  // Iabc control current in Amperes.

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void recalc();

	// configuration
	const float m_c;
	float m_max_iout_scale;
	float m_plus_scale;
	float m_minus_scale;
	sound_stream *m_stream;

	// state
	bool m_converged;
	float m_g;  // Caches the product of multiple factors. See recalc().
	float m_target_v;
	float m_iabc;
	float m_v;
};

#endif  // MAME_SOUND_VA_EG_H
