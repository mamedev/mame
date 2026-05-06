// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_PCFXGA_H
#define MAME_BUS_PC98_CBUS_PCFXGA_H

#pragma once

#include "slot.h"

class pcfxga_cbus_device : public device_t
						 , public device_pc98_cbus_slot_interface
{
public:
	pcfxga_cbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr flags_type emulation_flags() { return flags::NOT_WORKING; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	virtual void io_map(address_map &map) ATTR_COLD;

	u8 m_comms[0x10];
};

DECLARE_DEVICE_TYPE(PCFXGA_CBUS, pcfxga_cbus_device)

#endif // MAME_BUS_PC98_CBUS_PCFXGA_H
