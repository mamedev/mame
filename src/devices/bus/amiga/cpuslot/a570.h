// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A570

    DMAC based CD-ROM controller for the A500

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_A570_H
#define MAME_BUS_AMIGA_CPUSLOT_A570_H

#pragma once

#include "cpuslot.h"
#include "machine/at28c16.h"
#include "machine/dmac.h"

namespace bus::amiga::cpuslot {

class a570_device : public device_t, public device_amiga_cpuslot_interface
{
public:
	a570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_cpuslot_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void rst_w(int state) override;

private:
	void map(address_map &map) ATTR_COLD;

	required_device<amiga_dmac_rev2_device> m_dmac;
	required_ioport m_config;

	std::unique_ptr<uint16_t[]> m_ram;
};

} // namespace bus::amiga::cpuslot

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_A570, bus::amiga::cpuslot, a570_device)

#endif // MAME_BUS_AMIGA_CPUSLOT_A570_H
