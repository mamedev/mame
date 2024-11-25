// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_BUDDHA_H
#define MAME_BUS_AMIGA_ZORRO_BUDDHA_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "bus/ata/ataintf.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buddha_device

class buddha_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	// speed register
	uint16_t speed_r(offs_t offset, uint16_t mem_mask = ~0);
	void speed_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// ide register
	uint16_t ide_0_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_0_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide_0_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_0_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide_1_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_1_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide_1_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_1_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// interrupt register
	uint16_t ide_0_interrupt_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t ide_1_interrupt_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_interrupt_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ide_0_interrupt_w(int state);
	void ide_1_interrupt_w(int state);

	void mmio_map(address_map &map) ATTR_COLD;

	required_device<ata_interface_device> m_ata_0;
	required_device<ata_interface_device> m_ata_1;

	bool m_ide_interrupts_enabled;
	int m_ide_0_interrupt;
	int m_ide_1_interrupt;
};

} // namespace bus::amiga::zorro

// device type definition
DECLARE_DEVICE_TYPE_NS(ZORRO_BUDDHA, bus::amiga::zorro, buddha_device)

#endif // MAME_BUS_AMIGA_ZORRO_BUDDHA_H
