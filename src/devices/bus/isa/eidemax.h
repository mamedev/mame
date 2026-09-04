// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Promise EIDEMAX

    16-bit ISA Enhanced IDE controller

***************************************************************************/

#ifndef MAME_BUS_ISA_EIDEMAX_H
#define MAME_BUS_ISA_EIDEMAX_H

#pragma once

#include "bus/ata/ataintf.h"
#include "isa.h"


class eidemax_device : public device_t, public device_isa16_card_interface
{
public:
	eidemax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	uint16_t cs0_r(offs_t offset, uint16_t mem_mask);
	void cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t cs1_r(offs_t offset);
	void cs1_w(offs_t offset, uint8_t data);

	void ide_interrupt(int state);

	required_device<ata_interface_device> m_ata;
	required_ioport m_config;

	offs_t m_rom_base;
	uint16_t m_base_reg;
	uint8_t m_irq_level;
};

// device type declaration
DECLARE_DEVICE_TYPE(ISA16_EIDEMAX, eidemax_device)

#endif // MAME_BUS_ISA_EIDEMAX_H
