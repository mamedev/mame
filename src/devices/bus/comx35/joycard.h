// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 F&M Joycard emulation

**********************************************************************/

#ifndef MAME_BUS_COMX35_JOYCARD_H
#define MAME_BUS_COMX35_JOYCARD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_joy_device

class comx_joy_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_comx_expansion_card_interface overrides
	virtual uint8_t comx_io_r(offs_t offset) override;

private:
	required_ioport m_joy1;
	required_ioport m_joy2;
};


// device type definition
DECLARE_DEVICE_TYPE(COMX_JOY, comx_joy_device)


#endif // MAME_BUS_COMX35_JOYCARD_H
