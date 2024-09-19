// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NAMCO_NAMCO_C67_H
#define MAME_NAMCO_NAMCO_C67_H

#pragma once

#include "cpu/tms32025/tms32025.h"

// base class
class namco_c67_device : public tms32025_device
{
public:
	namco_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

};

DECLARE_DEVICE_TYPE(NAMCO_C67, namco_c67_device)



#endif // MAME_NAMCO_NAMCO_C67_H
