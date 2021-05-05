// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMF262_H
#define MAME_SOUND_YMF262_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opl.h"


// ======================> ymf262_device

DECLARE_DEVICE_TYPE(YMF262, ymf262_device);

class ymf262_device : public ymfm_device_base<ymfm::ymf262>
{
public:
	// constructor
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }
};

#endif // MAME_SOUND_YMF262_H
