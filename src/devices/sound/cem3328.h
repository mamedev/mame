// license:BSD-3-Clause
// copyright-holders:tim lindner

#ifndef MAME_SOUND_CEM3328_H
#define MAME_SOUND_CEM3328_H

#pragma once

#include "sound/va_vcf.h"

DECLARE_DEVICE_TYPE(CEM3328_LPF4, cem3328_lpf4_device)

// CEM3328 four pole low-pass VCF.
// freq CV: Voltage at pin 2 (frequency control), in volts.
// res CV: Voltage at pin 10 (resonance control), in volts (0 to +5V).
//
// Audio input: unity gain. The real chip has about 130x gain from its signal
// input pins (5/6) to the output, and is normally driven through an input
// attenuator. That gain is not modeled: scale the input route so that the
// input level equals the wanted output level (the datasheet's clean output
// swing is 6.5V p-p).

// Known inaccuracies:
// - The datasheet only describes the resonance control as a "reverse audio
//   taper" reaching oscillation at +3.2V (typ.). The curve used here is an
//   approximation of that description, and the behavior above the
//   oscillation threshold is an estimate.
// - All parameters are datasheet typical values. Real parts vary: initial
//   frequency by about +/-1/3 octave, scale factor 18.5 to 21.5mV/octave,
//   oscillation threshold 2.7 to 3.7V.
// - All four poles are treated alike. The fourth pole uses a small
//   capacitor (C_D, about 300pF) behind a 100x transconductor, which should
//   be equivalent to the other poles' 0.03uF, but this is not verified.
// - Distortion is that of va_lpf4_device and may not match the real chip.
// - Volume vs. resonance tailoring (pin 4 network) is modeled only as the
//   uncompensated case (pin 4 grounded): configure_bass_gain_comp() is left
//   at 0.
// - The output DC shift is not modeled.
class cem3328_lpf4_device : public va_lpf4_device
{
public:
	// c_p: pole capacitor (pins 1, 11, 12; datasheet example 0.033uF).
	// r_ref: resistor from pin 3 (I_REF) to V_EE.
	// v_ee: negative supply voltage (e.g. -8.0).
	cem3328_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, double c_p, double r_ref, double v_ee) ATTR_COLD;
	cem3328_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// Voltage at pin 10 at which the filter self-oscillates (datasheet: 2.7 to 3.7V, typ. 3.2V).
	static constexpr float RES_OSC_V = 3.2F;

protected:
	float cv_to_freq(float freq_cv) const override;
	float cv_to_res(float res_cv) const override;

private:
	float m_f0;  // Cutoff frequency at V_FREQ = 0.
};

#endif // MAME_SOUND_CEM3328_H
