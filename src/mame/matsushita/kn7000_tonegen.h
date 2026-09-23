// license:GPL2+
// copyright-holders:Felipe Sanches

// KN7000 tone generator (IC201/IC205).

#ifndef MAME_MATSUSHITA_KN7000_TONEGEN_H
#define MAME_MATSUSHITA_KN7000_TONEGEN_H

#pragma once

#include "kn_tonegen.h"

DECLARE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device)

// kn7000_tonegen_device -- register decode for IC201/IC205.
//
class kn7000_tonegen_device : public kn_tonegen_base_device
{
public:
	// 128 voices: 64 per chip, slot bit 6 selecting sub (IC205) or master (IC201).
	kn7000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: kn_tonegen_base_device(mconfig, KN7000_TONEGEN, tag, owner, clock, 128)
	{ }

	// Every tone-generator register write (both
	// TGs) is routed here from io_w, once the TG-enable gate is open.
	virtual void tg_write(int tg, uint16_t addr, uint16_t data) override;

};

#endif // MAME_MATSUSHITA_KN7000_TONEGEN_H
