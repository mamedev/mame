// license:GPL2+
// copyright-holders:Felipe Sanches

// KN6000/KN6500 tone generator.

#ifndef MAME_MATSUSHITA_KN6000_TONEGEN_H
#define MAME_MATSUSHITA_KN6000_TONEGEN_H

#pragma once

#include "kn_tonegen.h"

DECLARE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device)

class kn6000_tonegen_device : public kn_tonegen_base_device
{
public:
	// 64 voice slots in ONE chip (IC213) behind a single window -- no chip select,
	// so the driver always calls tg_write() with tg = 1 (the 0x98050000 window).
	kn6000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: kn_tonegen_base_device(mconfig, KN6000_TONEGEN, tag, owner, clock, 64)
	{ }

	virtual void tg_write(int tg, uint16_t addr, uint16_t data) override;
};

#endif // MAME_MATSUSHITA_KN6000_TONEGEN_H
