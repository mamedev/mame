// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer RAM Expansion Card

***************************************************************************/

#ifndef MAME_BUS_MC68000_RAM_H
#define MAME_BUS_MC68000_RAM_H

#pragma once

#include "sysbus.h"


class mc68000_ram_device : public device_t, public device_mc68000_sysbus_card_interface
{
public:
	// construction/destruction
	mc68000_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t slot_r(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void slot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	std::unique_ptr<uint16_t[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(MC68000_RAM, mc68000_ram_device)


#endif // MAME_BUS_MC68000_RAM_H
