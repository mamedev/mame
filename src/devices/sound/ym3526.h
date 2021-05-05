// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM3526_H
#define MAME_SOUND_YM3526_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opl.h"


// ======================> ym3526_device

DECLARE_DEVICE_TYPE(YM3526, ym3526_device);

class ym3526_device : public ymfm_device_base<ymfm::ym3526>
{
public:
	// constructor
	ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_YM3526_H
