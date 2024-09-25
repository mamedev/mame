// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_ISA_IDE_H
#define MAME_BUS_ISA_IDE_H

#pragma once

#include "isa.h"
#include "machine/idectrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_ide_device

class isa16_ide_device : public device_t,
	public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void cdrom_headphones(device_t *device);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ide_interrupt(int state);
	uint8_t ide16_alt_r();
	void ide16_alt_w(uint8_t data);
	bool is_primary() { return m_is_primary; }

	void map(address_map &map) ATTR_COLD;
	void alt_map(address_map &map) ATTR_COLD;

	// internal state
	bool m_is_primary;
	required_device<ide_controller_device> m_ide;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_IDE, isa16_ide_device)

#endif // MAME_BUS_ISA_IDE_H
