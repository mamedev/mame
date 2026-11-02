// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_IGS_IGS036_H
#define MAME_IGS_IGS036_H

#pragma once

#include "cpu/arm7/arm7.h"

class igs036_cpu_device : public arm946es_cpu_device
{
public:
	// construction/destruction
	igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void onboard_peripherals(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(IGS036,       igs036_cpu_device)

#endif // MAME_IGS_IGS027A_H
