// license:BSD-3-Clause
// copyright-holders:m1macrophage

// Curtis Electromusic Specialties CEM3372, uP-controllable signal processor.
//
// The chip consists of a VCF and VCA section. The output of the VCF is
// typically connected to the input of the VCA, often after further processing.
// The VCF section includes two inputs and a VCA for each.

#ifndef MAME_SOUND_CEM3372_H
#define MAME_SOUND_CEM3372_H

#pragma once

#include "sound/va_vca.h"
#include "sound/va_vcf.h"

DECLARE_DEVICE_TYPE(CEM3372_VCF, cem3372_vcf_device)
DECLARE_DEVICE_TYPE(CEM3372_VCA, cem3372_vca_device)


// VCF section of the CEM3372.
// A 2-channel mixer and 4-pole lowpass filter.
class cem3372_vcf_device : public device_t, public device_sound_interface
{
public:
	enum output_stream
	{
		// Final VCF output.
		OUTPUT_VCFOUT = 0,  // pin 17, voltage

		// OUTPUT_CAP{X} are the signals on each filter capacitor. Those are
		// unbuffered on the real device. Some synths use these to implement
		// multiple filter modes. The amplitude of the OUTPUT_CAP{X} signals is
		// 1/100 that of OUTPUT_VCFOUT.
		OUTPUT_CAP1,  // pin 4, voltage
		OUTPUT_CAP2,  // pin 3, voltage
		OUTPUT_CAP3,  // pin 2, voltage
		OUTPUT_CAP4,  // pin 1, voltage
	};

	struct stream_inputs
	{
		device_sound_interface *channel_a = nullptr;  // pin 6, voltage
		device_sound_interface *channel_b = nullptr;  // pin 7, voltage
		device_sound_interface *level_a = nullptr;  // pin 5, voltage
		device_sound_interface *level_b = nullptr;  // pin 8, voltage
		device_sound_interface *freq_cv = nullptr;  // pin 15, voltage
		device_sound_interface *res_cv = nullptr;  // pin 10, voltage
	};

	cem3372_vcf_device(const machine_config &mconfig, const char *tag, device_t *owner, const stream_inputs &inputs) ATTR_COLD;
	cem3372_vcf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	// Sets the capacitor values for each stage. Must be called for each stage
	// during configuration. Can be called at runtime as needed.
	// In a standard configuration, c1 = c2 = c3 and c4 = c3 / 100.
	cem3372_vcf_device &set_c1(float c1) { return set_c(0, c1); }
	cem3372_vcf_device &set_c2(float c2) { return set_c(1, c2); }
	cem3372_vcf_device &set_c3(float c3) { return set_c(2, c3); }
	cem3372_vcf_device &set_c4(float c4) { return set_c(3, c4); }

	// These setters can only be used if there is no corresponding stream input.
	void set_level_a_cv(float cv);  // pin 5, 0-5V
	void set_level_b_cv(float cv);  // pin 8, 0-5V
	void set_freq_cv(float cv);  // pin 15, -155mV - 110mV
	void set_res_cv(float cv);  // pin 10, 0-5V

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void sound_stream_update(sound_stream &stream) override;

private:
	cem3372_vcf_device &set_c(int stage, float c);

	float compute_level(float cv, int idx);
	float compute_freq(float cv);
	float compute_res(float cv);

	void update_freq_scale();

	const stream_inputs m_inputs;
	sound_stream *m_stream;

	required_device<va_vca_device> m_input_a_vca;
	required_device<va_vca_device> m_input_b_vca;
	required_device<va_lpf4_device> m_vcf;

	std::array<float, 4> m_c;
	std::array<float, 2> m_level_cv;
	float m_freq_cv;
	float m_res_cv;

	std::array<float, 2> m_level;
	float m_freq;
	float m_res;
};


// VCA section of the CEM3372.
//
// Input streams:
// - cem3372_vca_device::INPUT_AUDIO (required): Input to pin 12. A current in Amperes.
//
//   Typically, and as shown in the datasheet, the input audio signal (a voltage)
//   will be connected to a capacitor followed by a resistor (a dc-blocking HPF),
//   before being fed to pin 12. Pin 12 is held at -1V, so the resistor also
//   converts the voltage to a current. A typical connection to INPUT_AUDIO will
//   look something like (pseudocode):
//
//     FILTER_RC(...)
//         .set_rc(HIGHPASS, R, C)
//         .add_route(0, m_vca, 1.0 / R, INPUT_AUDIO)
//
//   Note the "1.0 / R" to convert the voltage to a current.
//
// - INPUT_GAIN (optional): Control voltage (0-5V) into pin 13. If this is not
//   configured, gain can be set using set_fix_gain_cv().
//
// Output stream: Current (in Amperes) at pin 14.
//
class cem3372_vca_device : public va_vca_device
{
public:
	cem3372_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	// By default, the streaming output will be the current at pin 14. This
	// function will configure the output to be a voltage. It will emulate an
	// inverting, op-amp-based current-to-voltage converter attached to pin 14,
	// as shown in the datasheet.
	cem3372_vca_device &configure_inv_opamp_voltage_output(float r_feedback) ATTR_COLD;

protected:
	void device_start() override ATTR_COLD;

	float cv_to_gain(float cv) const override;

private:
	float m_output_scale;  // Configuration. Not needed in save state.
};

#endif  // MAME_SOUND_CEM3372_H
