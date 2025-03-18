// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A590

    DMAC based SCSI controller for the A500

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_A590_H
#define MAME_BUS_AMIGA_CPUSLOT_A590_H

#pragma once

#include "cpuslot.h"
#include "bus/isa/hdc.h"
#include "machine/autoconfig.h"
#include "machine/dmac.h"
#include "machine/input_merger.h"
#include "machine/wd33c9x.h"


namespace bus::amiga::cpuslot {

class a590_device : public device_t, public device_amiga_cpuslot_interface
{
public:
	a590_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_cpuslot_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void rst_w(int state) override;

private:
	void wd33c93_config(device_t *device);

	uint8_t xt_r(offs_t offset);
	void xt_w(offs_t offset, uint8_t data);
	uint8_t dip_r(offs_t offset);

	required_device<input_merger_any_high_device> m_irq;
	required_device<amiga_dmac_rev2_device> m_dmac;
	required_device<wd33c93a_device> m_wdc;
	required_device<xt_hdc_device> m_xt;
	required_ioport m_jp1;
	required_ioport m_dip;

	std::unique_ptr<uint16_t[]> m_ram;
	uint8_t m_ram_size;
};

} // namespace bus::amiga::cpuslot

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_A590, bus::amiga::cpuslot, a590_device)

#endif // MAME_BUS_AMIGA_CPUSLOT_A590_H
