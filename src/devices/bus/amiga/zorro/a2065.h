// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2065

    Zorro-II Ethernet Network Interface

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_A2065_H
#define MAME_BUS_AMIGA_ZORRO_A2065_H

#pragma once

#include "zorro.h"
#include "machine/am79c90.h"
#include "machine/autoconfig.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2065_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	a2065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t host_ram_r(offs_t offset);
	void host_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t lance_ram_r(offs_t offset);
	void lance_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lance_irq_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_device<am7990_device> m_lance;

	std::unique_ptr<uint16_t[]> m_ram;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_A2065, bus::amiga::zorro, a2065_device)

#endif // MAME_BUS_AMIGA_ZORRO_A2065_H
