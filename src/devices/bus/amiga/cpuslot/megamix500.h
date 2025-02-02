// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    3-State MegaMix 500

    External RAM expansion for the A500

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_MEGAMIX500_H
#define MAME_BUS_AMIGA_CPUSLOT_MEGAMIX500_H

#pragma once

#include "cpuslot.h"
#include "machine/autoconfig.h"


namespace bus::amiga::cpuslot {

class megamix500_device : public device_t, public device_amiga_cpuslot_interface, public amiga_autoconfig
{
public:
	megamix500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_cpuslot_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void rst_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_ioport m_config;

	std::unique_ptr<uint16_t[]> m_ram;
	uint8_t m_ram_size;
	offs_t m_base_address;
};

} // namespace bus::amiga::cpuslot

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_MEGAMIX500, bus::amiga::cpuslot, megamix500_device)

#endif // MAME_BUS_AMIGA_CPUSLOT_MEGAMIX500_H
