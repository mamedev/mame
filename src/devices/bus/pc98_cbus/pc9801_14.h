// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_PC9801_14_H
#define MAME_BUS_PC98_CBUS_PC9801_14_H

#pragma once

#include "slot.h"

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/tms3631.h"

class pc9801_14_device : public device_t
					   , public device_pc98_cbus_slot_interface
{
public:
	pc9801_14_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<i8255_device> m_ppi;
	required_device<pit8253_device> m_pit;
	required_device<tms3631_device> m_tms;
	required_memory_region m_bios;
};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_14, pc9801_14_device)

#endif // MAME_BUS_PC98_CBUS_PC9801_14_H
