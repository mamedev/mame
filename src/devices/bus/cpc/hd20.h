// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Dobbertin HD20 hard disk

    Fixed disk interface for the Amstrad CPC

    Controller: Seagate ST11M XT HD Controller
    Disk: 3.5" 20MB Seagate, Kyocera, NEC or Miniscribe (Geometry: 615 cylinders/4 heads/17 sectors)

*/

#ifndef MAME_BUS_CPC_HD20_H
#define MAME_BUS_CPC_HD20_H

#pragma once

#include "cpcexp.h"
#include "bus/isa/hdc.h"

class cpc_hd20_device  : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_hd20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t hdc_r(offs_t offset);
	void hdc_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void irq_w(int state);

	cpc_expansion_slot_device *m_slot;

	required_device<xt_hdc_device> m_hdc;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_HD20, cpc_hd20_device)

#endif // MAME_BUS_CPC_HD20_H
