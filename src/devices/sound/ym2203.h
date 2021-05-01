// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2203_H
#define MAME_SOUND_YM2203_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm_opn.h"


// ======================> ym2203_device

DECLARE_DEVICE_TYPE(YM2203, ym2203_device);

class ym2203_device : public ymfm_device_ssg_base<ymfm::ym2203>
{
public:
	// constructor
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_YM2203_H
