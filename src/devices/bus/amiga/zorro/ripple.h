// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    RIPPLE IDE

    Zorro-II IDE interface for Amiga 2000/3000/4000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_RIPPLE_H
#define MAME_BUS_AMIGA_ZORRO_RIPPLE_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "machine/intelfsh.h"
#include "bus/ata/ataintf.h"


namespace bus::amiga::zorro {

class ripple_ide_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	ripple_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void busrst_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void mmio_map(address_map &map) ATTR_COLD;

	// flash
	void bank_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t banked_flash_r(offs_t offset);
	void banked_flash_w(offs_t offset, uint8_t data);

	// ide register
	uint16_t ide0_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide0_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide0_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide0_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide1_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide1_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide1_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide1_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	required_device<ata_interface_device> m_ata_0;
	required_device<ata_interface_device> m_ata_1;
	required_device<sst_39sf010_device> m_flash;
	memory_passthrough_handler m_write_tap;

	offs_t m_base_address = 0;
	uint8_t m_flash_bank = 0;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_RIPPLE, bus::amiga::zorro, ripple_ide_device)

#endif // MAME_BUS_AMIGA_ZORRO_RIPPLE_H
