// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM3812_H
#define MAME_SOUND_YM3812_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opl.h"


// ======================> ym3812_device

DECLARE_DEVICE_TYPE(YM3812, ym3812_device);

class ym3812_device : public ymfm_device_base<ymfm::ym3812>
{
public:
	// constructor
	ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_YM3812_H
