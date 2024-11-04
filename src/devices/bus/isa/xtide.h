// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
#ifndef MAME_BUS_ISA_XTIDE_H
#define MAME_BUS_ISA_XTIDE_H

#pragma once

#include "isa.h"
#include "bus/ata/ataintf.h"
#include "machine/eeprompar.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class xtide_device : public device_t, public device_isa8_card_interface
{
public:
	// construction/destruction
	xtide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void ide_interrupt(int state);

	required_device<ata_interface_device> m_ata;
	required_device<eeprom_parallel_28xx_device> m_eeprom;

	uint8_t m_irq_number;
	uint8_t m_d8_d15_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_XTIDE, xtide_device)

#endif // MAME_BUS_ISA_XTIDE_H
