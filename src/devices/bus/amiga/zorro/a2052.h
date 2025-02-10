// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2052

    Zorro-II RAM Expansion (0.5, 1 or 2 MB)

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_A2052_H
#define MAME_BUS_AMIGA_ZORRO_A2052_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2052_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	a2052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_ioport m_config;
	std::unique_ptr<uint16_t[]> m_ram;
	int m_ram_size;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_A2052, bus::amiga::zorro, a2052_device)

#endif // MAME_BUS_AMIGA_ZORRO_A2052_H
