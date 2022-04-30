// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPZ_H
#define MAME_SOUND_YMOPZ_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opz.h"


// ======================> opm_device_base

DECLARE_DEVICE_TYPE(YM2414, ym2414_device);

class ym2414_device : public ymfm_device_base<ymfm::ym2414>
{
	using parent = ymfm_device_base<ymfm::ym2414>;

public:
	// constructor
	ym2414_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_write_handler() { return io_write_handler(); }
};

#endif // MAME_SOUND_YMOPZ_H
