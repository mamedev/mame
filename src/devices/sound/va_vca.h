// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
Virtual analog amplifiers (VCAs).

The gain CV ("control value") can either be provided via a class method, or in
an input stream.

The meaning of "gain CV" depends on the (sub)class being instantiated. It will
typically match the type of gain control input in the emulated hardware.

Summary of devices. See class comments for more details.

* VA_VCA: An ideal, linear amplifier. The gain CV is just the gain (
  output = gain_cv * input).

* CEM3360_VCA: A CEM3360 OTA configured as a VCA. The gain CV is the voltage
  applied to certain pins, depending on configuration (see class documentation).
*/

#ifndef MAME_SOUND_VA_VCA_H
#define MAME_SOUND_VA_VCA_H

#pragma once

DECLARE_DEVICE_TYPE(VA_VCA, va_vca_device)
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

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	float m_fixed_gain;
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
