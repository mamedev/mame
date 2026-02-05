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

	va_lpf4_device &configure_input_gain(float gain) ATTR_COLD;

	// Larger values result in more distortion and more of the (filtered)
	// signal making it through at full resonance. The "correct" value will
	// depend on the device being emulated, and finding it might require
	// experimentation. A decent starting point is: 1 / (PP / 2), where PP is
	// the peak-to-peak magnitude of the input signal.
	// TODO: Revisit and/or find a better way to determine this.
	va_lpf4_device &configure_drive(float drive) ATTR_COLD;

	// The meaning of "CV" depends on the class being instantiated. See the
	// overview at the top of the file.
	void set_fixed_freq_cv(float freq_cv);
	void set_fixed_res_cv(float res_cv);

	float get_freq();  // Returns the cutoff frequency, in Hz.
	float get_res();  // Returns the feedback gain (0-4).

protected:
	va_lpf4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	virtual float cv_to_freq(float freq_cv) const;
	virtual float cv_to_res(float res_cv) const;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void recalc_alpha0();
	void recalc_filter();

	sound_stream *m_stream;

	// Configuration, not needed in save state.
	float m_input_gain;
	float m_drive;
	float m_drive_inv;

	// Filter state.
	float m_fc;  // Cutoff frequency in Hz.
	float m_res;  // Feedback gain.
	struct filter_stage
	{
		float alpha = 1;
		float beta = 0;
		float state = 0;
	};
	std::array<filter_stage, 4> m_stages;
	float m_alpha0;
	float m_G4;
};


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

#endif  // MAME_SOUND_VA_VCF_H
