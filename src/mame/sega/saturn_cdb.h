// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SEGA_SATURN_CDB_H
#define MAME_SEGA_SATURN_CDB_H

#pragma once

#include "cpu/sh/sh7032.h"

DECLARE_DEVICE_TYPE(SATURN_CDB, saturn_cdb_device)

class saturn_cdb_device : public device_t
{
public:
	// construction/destruction
	saturn_cdb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void saturn_cdb_map(address_map &map) ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:

};

#endif // MAME_SEGA_SATURN_CDB_H
