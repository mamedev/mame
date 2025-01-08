// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_BOOTBUG_H
#define MAME_BUS_NUBUS_BOOTBUG_H

#pragma once

#include "nubus.h"
#include "machine/ins8250.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_bootbug_device

class nubus_bootbug_device :
		public device_t,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_bootbug_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_bootbug_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<ns16450_device> m_uart;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	uint32_t dev_r(offs_t offset);
	void dev_w(offs_t offset, uint32_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_BOOTBUG, nubus_bootbug_device)

#endif // MAME_BUS_NUBUS_BOOTBUG_H
