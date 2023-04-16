// license:GPL-2.0+
// copyright-holders:Russell Bull
/*********************************************************************

    bml3rtc.h

    Hitachi RTC card for the MB-6890

*********************************************************************/

#ifndef MAME_BUS_BML3_BML3RTC_H
#define MAME_BUS_BML3_BML3RTC_H

#pragma once

#include "bml3bus.h"
#include "machine/msm5832.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_rtc_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t bml3_rtc_r(offs_t offset);
	void bml3_rtc_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void map_io(address_space_installer &space) override;

private:
	required_device<msm5832_device> m_rtc;

	uint8_t m_addr_latch;
	uint8_t m_data_latch;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_RTC, bml3bus_rtc_device)

#endif // MAME_BUS_BML3_BML3RTC_H
