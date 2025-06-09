// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_VCA_H
#define MAME_SOUND_VA_VCA_H

#pragma once

// Emulates a voltage-controled amplifier (VCA). The control value (CV) can
// be set directly (set_fixed_cv()), or it can be provided in a sound stream
// (input 1), by a device in va_eg.h, for example. When the cv is provided via a
// stream, this is also a ring modulator.
//
// The behavior of specific VCAs can be emulated by using the respective
// configure_* functions.
// Note that "CV" ("control value") could either refer to a control voltage, or
// a control current, depending on the device.
class va_vca_device : public device_t, public device_sound_interface
{
public:
	va_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// When streaming is enabled, the CV will be obtained from input 1 of the
	// sound stream. Otherwise, the value set with `set_fixed_cv` will be used.
	va_vca_device &configure_streaming_cv(bool use_streaming_cv);

	// By default, the CV will be treated as a typical gain (output = cv * input)
	// The configure_*() functions below might change this.

	// CEM3360 (or AS3360) VCA in linear CV configuration: CV connected to pin
	// Vc, and pins Vo and Ve connected to each other. The CV input (fixed or
	// streaming) should be the voltage at the Vc pin.
	va_vca_device &configure_cem3360_linear_cv();

	// Ignored when streaming CVs are enabled.
	void set_fixed_cv(float cv);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	float cv_to_gain(float cv) const;

	// Configuration. No need to include in save state.
	sound_stream *m_stream;
	bool m_has_cv_stream;
	float m_min_cv;
	float m_max_cv;
	float m_cv_scale;

	// State.
	float m_fixed_gain;
};

DECLARE_DEVICE_TYPE(VA_VCA, va_vca_device)

#endif  // MAME_SOUND_VA_VCA_H
