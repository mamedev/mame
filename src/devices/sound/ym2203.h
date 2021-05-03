// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2203_H
#define MAME_SOUND_YM2203_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm_opn.h"


// ======================> ym2203_device

DECLARE_DEVICE_TYPE(YM2203, ym2203_device);

class ym2203_device : public ymfm_device_base<ymfm::ym2203, ymfm::ym2203::SSG_OUTPUTS>
{
public:
	// constructor
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_a_read_callback() { return io_read_handler(0); }
	auto port_b_read_callback() { return io_read_handler(1); }
	auto port_a_write_callback() { return io_write_handler(0); }
	auto port_b_write_callback() { return io_write_handler(1); }
};

#endif // MAME_SOUND_YM2203_H
