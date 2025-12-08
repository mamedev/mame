// license:BSD-3-Clause
// copyright-holders:m1macrophage
/*
Virtual analog filters:

* VA_LPF4 / va_lpf4_device

    An ideal (linear) 4th order, resonant low-pass filter.

    Cutoff frequency and resonance can either be provided by calling class
    methods, or via input streams.

    The frequency CV is in Hz, and the resonance CV is the feedback gain (0-4).
    The meaning of CV can be different in subclasses: it will typically match
    the type of inputs in the emulated hardware.

* CEM3320_LPF4 / cem3320_lpf4_device:

    A CEM3320 configured as a 4th order low-pass filter, with optional resonance
    control.

    The frequency CV is the voltage applied to pin 12. The resonance CV is the
    voltage applied to resistor R_RC, connected to pin 9.
*/

#ifndef MAME_SOUND_VA_VCF_H
#define MAME_SOUND_VA_VCF_H

#pragma once

DECLARE_DEVICE_TYPE(VA_LPF4, va_lpf4_device)
DECLARE_DEVICE_TYPE(CEM3320_LPF4, cem3320_lpf4_device)

class va_lpf4_device : public device_t, public device_sound_interface
{
public:
	enum input_streams
	{
		INPUT_AUDIO = 0,
		INPUT_FREQ,
		INPUT_RES
	};

	va_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// The meaning of "CV" depends on the class being instantiated. See the
	// overview at the top of the file.
	void set_fixed_freq_cv(float freq_cv);
	void set_fixed_res_cv(float res_cv);

protected:
	va_lpf4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	virtual float cv_to_freq(float freq_cv) const;
	virtual float cv_to_res(float res_cv) const;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void recalc_filter();

	sound_stream *m_stream;

	float m_fc;  // Cutoff frequency in Hz.
	float m_res;  // Feedback gain.
	std::array<float, 5> m_a;
	std::array<float, 5> m_b;
	std::array<float, 4> m_x;
	std::array<float, 4> m_y;
};


// A CEM3320 configured as a 4th order lowpass filter, with optional resonance.
// freq CV: Voltage applied to pin 12.
// res CV: Voltage applied to resistor R_RC connected to pin 9.

// Known inaccuracies:
// - Filter implementation is linear.
// - On the actual device, once self-oscillation is achieved, increasing the
//   resonance CV will increase the amplitude of the oscillation (up to a limit).
//   This is not modeled here. The maximum resonance is capped for filter stability.
// - The resonance CV response was eyeballed from a graph on the datasheet, and
//   is approximate. Furthermore, that graph only goes up to a control current
//   of 300 uA. The implementation here extrapolates linearly beyond that.
class cem3320_lpf4_device : public va_lpf4_device
{
public:
	// c_p: pole capacitor. The value of the capacitors connected to pins 4, 5, 11, 16.
	// r_f: feedback resistor. The value of the resistors connecting pins 1 and 7,
	//      2 and 6, 17 and 15, and 18 and 10.
	cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, float c_p, float r_f) ATTR_COLD;
	cem3320_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

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
	// Configuration, not needed in save state.
	float m_r_eq;  // R_EQ in the datasheet.
	float m_cv2freq;  // Cached computation for frequency calculations.
	bool m_res_enabled;
	float m_r_rc;  // R_RC in the datasheet.
	float m_res_a;  // Cached computation for resonance calculations.
};

#endif  // MAME_SOUND_VA_VCF_H
