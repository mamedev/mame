// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Click cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Slogger_Click.html

***************************************************************************/

#ifndef MAME_BUS_BBC_CART_CLICK_H
#define MAME_BUS_BBC_CART_CLICK_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_click_device

class bbc_click_device : public device_t, public device_bbc_cart_interface
{
public:
	// construction/destruction
	bbc_click_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(click_button);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_CLICK, bbc_click_device)

#endif // MAME_BUS_BBC_CART_CLICK_H
