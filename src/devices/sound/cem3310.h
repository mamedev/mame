// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3310_H
#define MAME_SOUND_CEM3310_H

#pragma once

#include "sound/va_eg.h"

DECLARE_DEVICE_TYPE(CEM3310, cem3310_device)

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

#endif  // MAME_SOUND_CEM3310_H
