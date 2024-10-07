// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Dobbertin Smartwatch

    Dallas DS1216 Smartwatch + DS1315 Phantom Time chip

    Further info at: http://www.cpcwiki.eu/index.php/Dobbertin_Smart_Watch

*/

#ifndef MAME_BUS_CPC_SMARTWATCH_H
#define MAME_BUS_CPC_SMARTWATCH_H

#pragma once

#include "cpcexp.h"
#include "machine/ds1315.h"

class cpc_smartwatch_device : public device_t, public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_smartwatch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t rtc_w(offs_t offset);
	uint8_t rtc_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<ds1315_device> m_rtc;
	memory_bank* m_bank;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_SMARTWATCH, cpc_smartwatch_device)


#endif // MAME_BUS_CPC_SMARTWATCH_H
