// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SATURN_CDB_H
#define MAME_MACHINE_SATURN_CDB_H

#pragma once

#include "cpu/sh/sh2.h"

DECLARE_DEVICE_TYPE(SATURN_CDB, saturn_cdb_device)

class saturn_cdb_device : public device_t
{
public:
	// construction/destruction
	saturn_cdb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void saturn_cdb_map(address_map &map);
protected:
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:

};

#endif // MAME_MACHINE_SATURN_CDB_H
