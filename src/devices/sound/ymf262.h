// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMF262_H
#define MAME_SOUND_YMF262_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm_opl.h"


// ======================> ymf262_device

DECLARE_DEVICE_TYPE(YMF262, ymf262_device);

class ymf262_device : public ymfm_device_base<ymfm::ymf262>
{
public:
	// constructor
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif // MAME_SOUND_YMF262_H
