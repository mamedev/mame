// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun SunPC 5x86 Accelerator (501-4230) skeleton

***************************************************************************/

#ifndef MAME_BUS_SBUS_SUNPC_H
#define MAME_BUS_SBUS_SUNPC_H

#pragma once

#include "sbus.h"


class sbus_sunpc_device : public device_t, public device_sbus_card_interface
{
public:
	// construction/destruction
	sbus_sunpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	uint32_t unknown_r(offs_t offset, uint32_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rom_r(offs_t offset);

private:
	void mem_map(address_map &map) override;

	required_memory_region m_rom;
};


DECLARE_DEVICE_TYPE(SBUS_SUNPC, sbus_sunpc_device)

#endif // MAME_BUS_SBUS_SUNPC_H
