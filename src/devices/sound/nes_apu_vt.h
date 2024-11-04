// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_AUDIO_NES_VT_APU_H
#define MAME_AUDIO_NES_VT_APU_H

#pragma once

#include "sound/nes_apu.h"

DECLARE_DEVICE_TYPE(NES_APU_VT, nes_apu_vt_device)

class nes_apu_vt_device : public nesapu_device
{
public:
	// construction/destruction
	nes_apu_vt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
};

#endif // MAME_AUDIO_NES_VT_APU_H
