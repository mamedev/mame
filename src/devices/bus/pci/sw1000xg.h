// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_BUS_PCI_SW1000XG_H
#define MAME_BUS_PCI_SW1000XG_H

#pragma once

#include "ymp21.h"

#include "cpu/h8/h83002.h"
#include "sound/swp30.h"


class sw1000xg_device : public ymp21_device {
public:
	sw1000xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<h83002_device> m_maincpu;
	required_device<swp30_device> m_swp30;

	void h8_map(address_map &map) ATTR_COLD;
	void swp30_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SW1000XG, sw1000xg_device)

#endif
