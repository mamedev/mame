// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3320_H
#define MAME_SOUND_CEM3320_H

#pragma once

#include "sound/va_vcf.h"

DECLARE_DEVICE_TYPE(CEM3320_LPF4, cem3320_lpf4_device)

// A CEM3320 configured as a 4th order lowpass filter, with optional resonance.
// freq CV: Voltage applied to pin 12.
// res CV: Voltage applied to resistor R_RC connected to pin 9.

// Known inaccuracies:
// - While the implementation includes distortion, it might not match that of
//   the real chip.
// - The resonance CV response was eyeballed from a graph on the datasheet, and
//   is approximate. Furthermore, that graph only goes up to a control current
//   of 300 uA. The implementation here extrapolates linearly beyond that.
// - The output of the CEM3320 has a DC offset, which is not modeled here.
class cem3320_lpf4_device : public va_lpf4_device
{
public:
	// c_p: pole capacitor. The value of the capacitors connected to pins 4, 5, 11, 16.
	cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, double c_p) ATTR_COLD;
	cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	// Pin 1 (audio input) on the real device is a current input. But a voltage
	// can be provided via a resistor to pin 1 (R_I in the schematic).
	// Similarly, this implementation expects a current in the audio input steam,
	// by default. This method converts the input stream to a voltage input.
	cem3320_lpf4_device &configure_voltage_input(float r_i) ATTR_COLD;

	// Enable resonance (pin 10 connected to pin 8, via passive components)
	// using the configuration in the datasheet.
	// r_rc: The resistor connected to the resonance control input (pin 9).
	//       R_RC in the datasheet.
	// r_ri: The resistor between the filter output and the resonance signal
	//       input (pin 8). R_RI in the datasheet.
	cem3320_lpf4_device &configure_resonance(float r_rc, float r_ri) ATTR_COLD;

	// Similar to the above, but accommodates additional external circuitry that
	// affects the filter's coefficients.
	// r_ri_gnd: Some designs add a resistor from the resonance signal input
	//           (pin 8) to ground, possibly via a DC-blocking capacitor. Set
	//           to a negative number if no such resistor exists.
	// external_gain: Gain applied to the filter output (pin 10) before routing
	//                to the resonance signal input (pin 8).
	cem3320_lpf4_device &configure_resonance(float r_rc, float r_ri, float r_ri_gnd, float external_gain) ATTR_COLD;

protected:
	float cv_to_freq(float freq_cv) const override;
	float cv_to_res(float res_cv) const override;

private:
	static const float R_EQ;  // R_EQ in the datasheet.

	// Configuration, not needed in save state.
	float m_cv2freq;  // Cached computation for frequency calculations.
	bool m_res_enabled;
	float m_r_rc;  // R_RC in the datasheet.
	float m_res_a;  // Cached computation for resonance calculations.
};

#endif  // MAME_SOUND_CEM3320_H
