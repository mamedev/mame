// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MyAB Turbo-Kontroller disk controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_TURBO_H
#define MAME_BUS_ABCBUS_TURBO_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> turbo_kontroller_device

class turbo_kontroller_device :  public device_t,
									public device_abcbus_card_interface
{
public:
	// construction/destruction
	turbo_kontroller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type TURBO_KONTROLLER;
DECLARE_DEVICE_TYPE(TURBO_KONTROLLER, turbo_kontroller_device)

#endif // MAME_BUS_ABCBUS_TURBO_H
