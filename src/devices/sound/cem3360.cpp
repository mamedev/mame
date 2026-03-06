// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3360.h"

cem3360_vca_device::cem3360_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vca_device(mconfig, CEM3360_VCA, tag, owner, clock)
{
}

float cem3360_vca_device::cv_to_gain(float cv) const
{
	// Typical linear CV for max gain, as reported on the CEM3360 datasheet.
	constexpr float CEM3360_MAX_GAIN_CV = 1.93F;
	return std::clamp(cv, 0.0F, CEM3360_MAX_GAIN_CV) / CEM3360_MAX_GAIN_CV;
}

DEFINE_DEVICE_TYPE(CEM3360_VCA,    cem3360_vca_device,    "cem3360_vca",    "CEM3360-based VCA")
