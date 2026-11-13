// license:BSD-3-Clause
// copyright-holders:m1macrophage

// CEM3374 dual triangle-core voltage-controlled oscillator.

#ifndef MAME_SOUND_CEM3374_H
#define MAME_SOUND_CEM3374_H

#pragma once

#include "sound/va_vco.h"

DECLARE_DEVICE_TYPE(CEM3374, cem3374_device)


// One of the 2 oscillators in the CEM3374, which generates triangle and
// sawtooth waveforms, and an optional external comparator for generating the
// pulse waveform.
//
// The comparator logic needs to be included here, instead of as a downstream
// sound device, in order to achieve anti-aliasing.
//
// The two oscillators only differ in how sync is implemented. A sync on
// oscillator A will cause the underlying triangle waveform to reset to zero,
// whereas a sync on oscillator B will cause it to reverse direction. The
// sawtooth is derived from the triangle, so for oscillator A it will reset to
// zero, and for oscillator B it will "flip" its phase.
//
// Inputs:
//
// - Frequency control: set_freq_ctrl() or INPUT_FREQ_CTRL stream.
//   Control voltage (CV) at pin 7 (osc A) or pin 12 (osc B). The CV generated
//   by the synth is typically scaled with a voltage divider before being fed
//   to this pin. Remember to apply that scaling before passing to this device.
//
// - Reference current: set_freq_ref() or INPUT_FREQ_REF stream.
//   Reference current (Iref) at pin 8 (osc A) or pin 11 (osc B). This pin is
//   held at 0V, so in the common case where the circuit is:  V---R---pin 8,
//   Iref = V / R.
//
// - Pulse width (PW) control: set_pw_ctrl() or INPUT_PW_CTRL stream.
//   The pulse width voltage threshold set on the external pulse comparator. See
//   configure_pulse_cmp().
//
class cem3374_device : public va_vco_device
{
public:
	enum class osc : s8 { A = 0, B };

	// osc_side - Oscillator A or B?
	// vcc - Voltage at pin 18.
	// c_f - Capacitor at pin 6 (osc A) or pin 13 (osc B).
	//
	// NOTE: some designs have a resistor and a parallel capacitor connected
	// between c_f and GND. These are used for high-frequency scale trimming,
	// which is not yet emulated. They can be ignored for now.
	cem3374_device(const machine_config &mconfig, const char *tag, device_t *owner, osc osc_side, float vcc, float c_f) ATTR_COLD;
	cem3374_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	// Sets the threshold for the top of the triangle waveform. This affects
	// the amplitude of the two waveforms, and the frequency for a given control
	// input.
	// Only use if the V_TH input is connected to a voltage reference. Not
	// needed if the input is just connected to a capacitor, since the chip will
	// generate its own threshold in that case.
	cem3374_device &configure_vthresh(float vthresh) ATTR_COLD;  // pin 4 (osc A) or pin 15 (osc B)

	// Configures an external comparator that generates the pulse waveform, if
	// one exists. This compares one of the oscillator's waveforms (see (2)
	// below) with the pulse-width control.
	//
	// src_gain - gain applied to the source waveform before comparing with the
	//            PW control.
	//
	// If calling this function, chances are you also need to call:
	// (1) va_vco_device::configure_pulse_range() to specify the voltage levels
	//     of the pulse waveform.
	// (2) va_vco_device::configure_pulse_from_tri(), if the pulse is derived
	//     from the triangle output.
	cem3374_device &configure_pulse_cmp(float src_gain) ATTR_COLD;

	float v_tempco() const;  // pin 10, V_TC

protected:
	void device_start() override ATTR_COLD;

	float ctrl2freq(float freq_ctrl, float freq_ref) const override;
	float ctrl2pw(float pw_ctrl) const override;

private:
	const float m_vcc;
	const float m_cf;

	// Configuration, not needed in save state.
	float m_vthresh;
	float m_pulse_src_gain;
};

#endif  // MAME_SOUND_CEM3374_H
