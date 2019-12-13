// license:GPL-2.0+
// copyright-holders:Dirk Best
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


namespace bus { namespace amiga { namespace zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a2065_device

class a2065_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	a2065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( host_ram_r );
	DECLARE_WRITE16_MEMBER( host_ram_w );

	DECLARE_READ16_MEMBER( lance_ram_r );
	DECLARE_WRITE16_MEMBER( lance_ram_w );
	DECLARE_WRITE_LINE_MEMBER( lance_irq_w );

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( cfgin_w ) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_device<am7990_device> m_lance;

	std::unique_ptr<uint16_t[]> m_ram;
};

} } } // namespace bus::amiga::zorro

// device type definition
DECLARE_DEVICE_TYPE_NS(ZORRO_A2065, bus::amiga::zorro, a2065_device)

#endif // MAME_BUS_AMIGA_ZORRO_A2065_H
