// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2091

    DMAC based SCSI controller for the A2000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_A2091_H
#define MAME_BUS_AMIGA_ZORRO_A2091_H

#pragma once

#include "zorro.h"
#include "bus/isa/hdc.h"
#include "machine/autoconfig.h"
#include "machine/dmac.h"
#include "machine/input_merger.h"
#include "machine/wd33c9x.h"


namespace bus::amiga::zorro {

class a2091_device : public device_t, public device_zorro2_card_interface
{
public:
	a2091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void busrst_w(int state) override;

private:
	void wd33c93_config(device_t *device);

	uint8_t xt_r(offs_t offset);
	void xt_w(offs_t offset, uint8_t data);

	required_device<input_merger_any_high_device> m_irq;
	required_device<amiga_dmac_rev2_device> m_dmac;
	required_device<wd33c93a_device> m_wdc;
	required_device<xt_hdc_device> m_xt;
	required_ioport m_jp1;

	std::unique_ptr<uint16_t[]> m_ram;
	uint8_t m_ram_size;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_A2091, bus::amiga::zorro, a2091_device)

#endif // MAME_BUS_AMIGA_ZORRO_A2091_H
