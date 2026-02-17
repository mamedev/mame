// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3360_H
#define MAME_SOUND_CEM3360_H

#pragma once

#include "sound/va_vca.h"

DECLARE_DEVICE_TYPE(CEM3360_VCA, cem3360_vca_device)

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
// - Linear response.
// - Exponential gain control is not implemented.
class cem3360_vca_device : public va_vca_device
{
public:
	cem3360_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

protected:
	float cv_to_gain(float cv) const override;
};

#endif  // MAME_SOUND_CEM3360_H
