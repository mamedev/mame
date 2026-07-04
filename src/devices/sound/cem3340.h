// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3340_H
#define MAME_SOUND_CEM3340_H

#pragma once

#include "sound/va_vco.h"

DECLARE_DEVICE_TYPE(CEM3340, cem3340_device)

// CEM3340 voltage-controlled oscillator.
//
// A triangle-core oscillator with triangle, ramp and variable width pulse outputs.
//
// The frequency is controlled by a control current (in Amperes) to pin 15.
// The pulse width is controlled by a control voltage (in Volts) to pin 5.
//
// The CEM3340 supports multiple types of sync, some of which are unusual. At
// the moment, only the "conventional hard sync" setup is emulated (figure 5 in
// the datasheet), which is one that's commonly used. In this mode, a negative
// pulse will unconditionally switch the direction of the triangle wave, from
// which the other waveforms are derived from. However, we can't use the pulse
// as an input here. Instead, the sync oscillator's frequency should be provided
// as a stream input (see va_vco_device class documentation for details).
//
// TODO:
// - Other types of sync.
// - Linear FM input.
// - High frequency tracking input.
// - A lot of external components are hardcoded to the recommended values. Make
//   them configurable as needed.
class cem3340_device : public va_vco_device
{
public:
	// cf - timing capacitor between pin 11 and GND.
	// rr - reference current resistor between pin 13 and V+.
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, float cf, float rr) ATTR_COLD;
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

protected:
	void device_start() override ATTR_COLD;

	// Converts the control current at pin 15 to Hz.
	float ctrl2freq(float freq_ctrl) const override;

	// Converts the control voltage at pin 5 to a pulse width fraction (0-1).
	float ctrl2pw(float pw_ctrl) const override;

private:
	const float m_cf;
	const float m_rr;
};

#endif  // MAME_SOUND_CEM3340_H
