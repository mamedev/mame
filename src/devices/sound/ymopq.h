// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPQ_H
#define MAME_SOUND_YMOPQ_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opq.h"


// ======================> ym3806_device

DECLARE_DEVICE_TYPE(YM3806, ym3806_device);

class ym3806_device : public ymfm_device_base<ymfm::ym3806>
{
public:
	// constructor
	ym3806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ym3533_device

DECLARE_DEVICE_TYPE(YM3533, ym3533_device);

class ym3533_device : public ymfm_device_base<ymfm::ym3533>
{
public:
	// constructor
	ym3533_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_YMOPQ_H
