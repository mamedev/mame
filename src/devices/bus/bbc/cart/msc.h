// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Master Smart Cartridge emulation

***************************************************************************/

#ifndef MAME_BUS_BBC_CART_MSC_H
#define MAME_BUS_BBC_CART_MSC_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_msc_device

class bbc_msc_device : public device_t, public device_bbc_cart_interface
{
public:
	// construction/destruction
	bbc_msc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(activate);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_ioport m_button;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_MSC, bbc_msc_device)

#endif // MAME_BUS_BBC_CART_MSC_H
