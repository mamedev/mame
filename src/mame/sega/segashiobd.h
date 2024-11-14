// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_SEGA_SEGASHIOBD_H
#define MAME_SEGA_SEGASHIOBD_H

#pragma once

#include "cpu/sh/sh4.h"

DECLARE_DEVICE_TYPE(SEGA837_14438, sega_837_14438_device)

class sega_837_14438_device : public device_t
{
public:
	// construction/destruction
	sega_837_14438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void sh4_map(address_map &map) ATTR_COLD;

private:
	required_device<sh4_device> m_maincpu;
};

#endif // MAME_SEGA_SEGASHIOBD_H
