// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Click cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Slogger_Click.html

***************************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_CLICK_H
#define MAME_BUS_ELECTRON_CART_CLICK_H

#pragma once

#include "slot.h"
#include "machine/mc146818.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_click_device

class electron_click_device : public device_t,
								public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_click_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(click_button);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_device<mc146818_device> m_rtc;

	uint8_t m_page_register;
};

// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_CLICK, electron_click_device)

#endif // MAME_BUS_ELECTRON_CART_CLICK_H
