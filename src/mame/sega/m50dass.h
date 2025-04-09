// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_SEGA_M50DASS_H
#define MAME_SEGA_M50DASS_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

class m50dass_device : public device_t
{
public:
	m50dass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void m50dass(machine_config &config) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ym2413_device> m_ym2413;
};

DECLARE_DEVICE_TYPE(MEGALO50_DASS, m50dass_device)

#endif // MAME_SEGA_M50DASS_H
