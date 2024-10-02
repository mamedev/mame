// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Solidisk Real Time Clock emulation

    PMS Genie Watch (RTC for the BBC)

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_RTC_H
#define MAME_BUS_BBC_ROM_RTC_H

#pragma once

#include "slot.h"
#include "machine/mc146818.h"
#include "machine/ds1315.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_stlrtc_device

class bbc_stlrtc_device : public device_t,
							public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_stlrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;

private:
	required_device<mc146818_device> m_rtc;
};

// ======================> bbc_pmsrtc_device

class bbc_pmsrtc_device : public device_t,
	public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_pmsrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;

private:
	required_device<ds1315_device> m_rtc;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_STLRTC, bbc_stlrtc_device)
DECLARE_DEVICE_TYPE(BBC_PMSRTC, bbc_pmsrtc_device)

#endif // MAME_BUS_BBC_ROM_RTC_H
