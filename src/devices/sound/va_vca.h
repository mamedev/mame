// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
Virtual analog amplifiers (VCAs).

The gain CV ("control value") can either be provided via a class method, or in
an input stream.

The meaning of "gain CV" depends on the (sub)class being instantiated. It will
typically match the type of gain control input in the emulated hardware.

The device classes for specific chips expect voltages or currents as inputs.
Similarly, their outputs are voltages or currents, depending on device and
configuration. If a driver author does not require that level of accuracy, for
instance during early stage development, they should use VA_VCA.

Summary of devices. See class comments for more details.

* VA_VCA: An ideal, linear amplifier. The gain CV is just the gain (
  output = gain_cv * input).

* CA3280_VCA: CA3280-based VCA in non-linearized configuration (Id pin connected
  to the negative supply, or not connected at all). The gain CV is the control
  current supplied to the Iabc pin.

* CA3280_LIN_VCA: CA3280-based VCA in linearized configuration (Id pin connected
  to a current source, or to a resistor to V+). The gain CV is the control
  current supplied to the Iabc pin.
*/

#ifndef MAME_SOUND_VA_VCA_H
#define MAME_SOUND_VA_VCA_H

#pragma once

DECLARE_DEVICE_TYPE(VA_VCA, va_vca_device)
DECLARE_DEVICE_TYPE(CA3280_VCA, ca3280_vca_device)
DECLARE_DEVICE_TYPE(CA3280_VCA_LIN, ca3280_vca_lin_device)


// Emulates a voltage-controlled amplifier (VCA). The control value (CV) can
// be set directly (set_fixed_gain_cv()), or it can be provided in a sound stream
// (input 1), by a device in va_eg.h, for example. When the cv is provided via a
// stream, this is also a ring modulator.
//
// This can also emulate a differential amplifier, by supplying a fixed or
// streaming inverting input.
//
// The CV for this device is simply the gain (output = gain * input).
class va_vca_device : public device_t, public device_sound_interface
{
public:
	enum input_streams
	{
		INPUT_AUDIO = 0,
		INPUT_GAIN,
		INPUT_AUDIO_INV,
	};

	va_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// The meaning of "gain CV" depends on the class being instantiated. See
	// each (sub)class documentation.
	void set_fixed_gain_cv(float gain_cv);

	// Sets the inverting (-) input to a fixed value. Defaults to 0.
	void set_fixed_inv_input(float x);

	// Applies input differencing. This happens before gain is applied.
	// Subclasses can override diff() to apply additional processing such as
	// prescaling and distortion.
	// Public because it is useful for calibrating and debugging.
	// p ~ non-inverting input ("plus").
	// m ~ inverting input ("minus").
	virtual float diff(float p, float m) const;

protected:
	va_vca_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	void update();

	virtual float cv_to_gain(float cv) const;
	virtual u32 preferred_sample_rate() const;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	float m_fixed_gain;
	float m_fixed_inv_input;
};


// Emulates a CA3280-based VCA with its Id pin connected to V-, or not connected
// at all. For the linearized version (Id pin in use), see ca3280_vca_lin_device
// instead.
//
// In this configuration, the CA3280 has a tanh response, and can only be
// considered linear for very small input voltages (~[-0.02 - 0.02]). See
// figure 3B in the datasheet.
//
//                           ______    Iabc
//                          |      \    |
// A --- Rin+ ---+--- B --- |+      \   |
//               |          |        \  |
//             Rgnd+        |         \ |
//               |          |          OO---- F ----+
//              GND         |         /             |
//                          |        /             Rout
// [D -- Rin- --]+--- E --- |-  Id  /               |
//               |          |___|__/               GND
//             Rgnd-            |
//               |              |
//              GND             V-
//
// The streaming input should either be the voltage at point B, or at point A,
// depending on how the object is configured.
//
// An (optional) inverting input (fixed or streaming) is also supported. It
// should be the voltage at point E, or at point D, depending on configuration.
//
// The gain CV (fixed or streaming) should be the control current (Iabc), in
// Amperes.
//
// The streaming output will be the output current at point F (the CA3280 is a
// current source), but the object can be configured to output a voltage instead.
//
class ca3280_vca_device : public va_vca_device
{
public:
	ca3280_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// By default, the inputs should be the voltages at points B and E.
	// A common setup is to have voltage dividers at the input(s), which can be
	// configured by these methods. If dividers are configured, the inputs
	// should be the voltages at points A and D.
	ca3280_vca_device &configure_plus_divider(float r_in, float r_gnd) ATTR_COLD;
	ca3280_vca_device &configure_minus_divider(float r_in, float r_gnd) ATTR_COLD;

	// By default, the streaming output will be the current at point F. This
	// method will configure the output to be the voltage at point F.
	ca3280_vca_device &configure_voltage_output(float r_out) ATTR_COLD;

	float diff(float p, float m) const override;

protected:
	float cv_to_gain(float cv) const override;
	u32 preferred_sample_rate() const override;

private:
	// Configuration. Not needed in save state.
	float m_plus_scale;
	float m_minus_scale;
	float m_output_scale;
};


// Emulates a linearized CA3280, a configuration where the Id pin is connected
// to a current source, often a resistor to the positive supply.
//
// In this configuration, the CA3280 is linear for a wide voltage range, but it
// will hard-clip when Iin approaches Id (TODO: clipping is not currently
// emulated). See figure 3A in the datasheet.
//
//                       ______    Iabc
//                      |      \    |
//     A ---- Rin+ ---- |+      \   |
//                      |        \  |
//                      |         \ |
//                      |          OO---- F ----+
//                      |         /             |
//                      |        /             Rout
//     B ---- Rin- ---- |-  Id  /               |
//                      |___|__/               GND
//                          |
//                          + ---- Rd --- V+
//
// Usually, Rin+ = Rin- and B = GND. When that's the case:
// In = V_A / Rin+ and Iout = In * 0.82 * Iabc / Id.
//
// The gain CV (fixed or streaming) should be the control current (Iabc), in
// Amperes.
//
// The streaming output will be the output current at point F (the CA3280 is a
// current source), but the object can be configured to output a voltage instead.
//
class ca3280_vca_lin_device : public va_vca_device
{
public:
	// Use this constructor if Id is connected to a constant current source.
	ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, float i_d) ATTR_COLD;

	// Use this constructor if Id is connected to a resistor to some voltage source.
	// r: resistor connected to pin Id.
	// v_r: voltage on the other side of the resistor, typically the positive
	//      supply voltage to the chip.
	// v_minus: negative supply voltage to the chip.
	ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, float r, float v_r, float v_minus) ATTR_COLD;

	// Don't use.
	ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	// By default, the streaming output will be the current at point F. This
	// method will configure the output to be the voltage at point F.
	ca3280_vca_lin_device &configure_voltage_output(float r_out) ATTR_COLD;

	// These set Rin+ and Rin-. They must be called at least once and they
	// can change at runtime.
	// If the input voltage is applied though a resistor network, such as a
	// trimming circuit, use the Thevenin-equivalent resistance. Remember to
	// also feed the Thevenin-equivalent voltage to the input(s) in that case.
	ca3280_vca_lin_device &set_rplus(float r);
	ca3280_vca_lin_device &set_rminus(float r);

	float diff(float p, float m) const override;

protected:
	float cv_to_gain(float cv) const override;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	void update_cv_scale();

	static float i_d(float r, float v_r, float v_minus) ATTR_COLD;

	// Configuration. Not needed in save state.
	const float m_id;
	float m_output_scale;
	float m_cv_scale;

	// state
	float m_rp;
	float m_rm;
	float m_r_inv;
};

#endif  // MAME_SOUND_VA_VCA_H
