// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1110 8K RAM Expansion Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1110_H
#define MAME_BUS_VIC20_VIC1110_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1110_device

class vic1110_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
	required_ioport m_sw;

	vic20_expansion_window *m_window;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1110, vic1110_device)

#endif // MAME_BUS_VIC20_VIC1110_H
