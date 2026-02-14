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

* CEM3360_VCA: A CEM3360 OTA configured as a VCA. The gain CV is the voltage
  applied to certain pins, depending on configuration (see class documentation).
*/

#ifndef MAME_SOUND_VA_VCA_H
#define MAME_SOUND_VA_VCA_H

#pragma once

DECLARE_DEVICE_TYPE(VA_VCA, va_vca_device)
DECLARE_DEVICE_TYPE(CA3280_VCA, ca3280_vca_device)
DECLARE_DEVICE_TYPE(CA3280_VCA_LIN, ca3280_vca_lin_device)
DECLARE_DEVICE_TYPE(CEM3360_VCA, cem3360_vca_device)


// Emulates a voltage-controlled amplifier (VCA). The control value (CV) can
// be set directly (set_fixed_gain_cv()), or it can be provided in a sound stream
// (input 1), by a device in va_eg.h, for example. When the cv is provided via a
// stream, this is also a ring modulator.
//
// The CV for this device is simply the gain (output = gain * input).
class va_vca_device : public device_t, public device_sound_interface
{
public:
	enum input_streams
	{
		INPUT_AUDIO = 0,
		INPUT_GAIN,
	};

	va_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// The meaning of "gain CV" depends on the class being instantiated. See
	// each (sub)class documentation.
	void set_fixed_gain_cv(float gain_cv);

protected:
	va_vca_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	virtual float cv_to_gain(float cv) const;
	virtual float distorted(float s) const;
	virtual bool has_distortion() const { return false; }

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	float m_fixed_gain;
};


// Emulates a CA3280-based VCA with its Id pin connected to V-, or not connected
// at all. For the linearized version (Id pin in use), see ca3280_vca_lin_device
// instead.
//
// In this configuration, the CA3280 has a tanh response, and can only be
// considered linear for very small input voltages (~[-0.02 - 0.02]). See
// figure 3B in the datasheet.
//
//                          ______    Iabc
//                         |      \    |
// A --- Rin ---+--- B --- |+      \   |
//              |          |        \  |
//             Rgnd        |         \ |
//              |          |          OO---- C ----+
//             GND         |         /             |
//                         |        /             Rout
//              +--------- |-  Id  /               |
//              |          |___|__/               GND
//             Rgnd            |
//              |              |
//             GND             V-
//
// The streaming input should either be the voltage at point B, or at point A,
// depending on how the object is configured.
//
// The gain CV (fixed or streaming) should be the control current (Iabc), in
// Amperes.
//
// The streaming output will be the output current at point C (the CA3280 is a
// current source), but the object can be configured to output a voltage instead.
//
class ca3280_vca_device : public va_vca_device
{
public:
	ca3280_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// By default, the streaming input should be the voltage at point B.
	// This method will configure the input to be the voltage at point A.
	ca3280_vca_device &configure_input_divider(float r_in, float r_gnd) ATTR_COLD;

	// By default, the streaming output will be the current at point C. This
	// method will configure the output to be the voltage at point C.
	ca3280_vca_device &configure_voltage_output(float r_out) ATTR_COLD;

protected:
	float cv_to_gain(float cv) const override;
	float distorted(float s) const override;
	bool has_distortion() const override { return true; }

private:
	// Configuration. Not needed in save state.
	float m_input_scale;
	float m_output_scale;
};


// Emulates a linearized CA3280, a configuration where the Id pin is connected
// to a current source, often a resistor to the positive supply.
//
// In this configuration, the CA3280 is linear for a wide voltage range, but it
// will hard-clip when Iout approaches Iabc (TODO: clipping is not currently
// emulated). See figure 3A in the datasheet.
//
// Note that, in contrast to the non-linearized configuration above, it is more
// appropriate to treat the inputs as current inputs, rather than voltage inputs.
//
//                       ______    Iabc
//                      |      \    |
// A --- Rin+ --- B --- |+      \   |
//                      |        \  |
//                      |         \ |
//                      |          OO---- C ----+
//                      |         /             |
//                      |        /             Rout
//     GND --- Rin- --- |-  Id  /               |
//                      |___|__/               GND
//                          |
//                          + ---- Rd --- V+
//
// The streaming input should be the *current* at point B (the + input is at near
// ground potential). But it can be configured to be the voltage at point A.
//
// The gain CV (fixed or streaming) should be the control current (Iabc), in
// Amperes.
//
// The streaming output will be the output current at point C (the CA3280 is a
// current source), but the object can be configured to output a voltage instead.
//
// This implementation assumes Rin- == Rin+, which is typical.
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

	// By default, the streaming input should be the *current* at point B.
	// This method will configure the input to be the voltage at point A.
	ca3280_vca_lin_device &configure_voltage_input(float r_in) ATTR_COLD;

	// By default, the streaming output will be the current at point C. This
	// method will configure the output to be the voltage at point C.
	ca3280_vca_lin_device &configure_voltage_output(float r_out) ATTR_COLD;

protected:
	float cv_to_gain(float cv) const override;

private:
	void update_cv_scale();

	static float i_d(float r, float v_r, float v_minus) ATTR_COLD;

	// Configuration. Not needed in save state.
	const float m_i_d_inv;
	float m_input_scale;
	float m_output_scale;
	float m_cv_scale;
};


// Emulates a CEM3360-based VCA. Each CEM3360 has two OTAs that can be
// configured as VCAs.
//
// The CEM3360 can be configured for linear or exponential gain control.
// In linear control mode, pin Vo (4, 11) is connected to pin Ve (3, 12), and
// the CV is applied to pin Vc (5, 10).
// In exponential mode, pins Vo and Vc do not connect to anything, and the CV
// is applied to pin Ve, which is also connected to pin Vref (8) via a resistor.
//
// Currently, only the linear control mode is implemented. The provided CV is
// the voltage applied to pin Vc (5, 10).
//
// Known inaccuracies:
// - Linear response. The real device likely has a tanh response, given it is
//   OTA-based.
// - Exponential gain control is not implemented.
class cem3360_vca_device : public va_vca_device
{
public:
	cem3360_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

protected:
	float cv_to_gain(float cv) const override;
};

#endif  // MAME_SOUND_VA_VCA_H
