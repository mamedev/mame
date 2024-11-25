// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System - Bandai Power Pad

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_POWERPAD_H
#define MAME_BUS_NES_CTRL_POWERPAD_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_powerpad_device

class nes_powerpad_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_powerpad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit34() override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<2> m_ipt;
	u8 m_latch[2];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_POWERPAD, nes_powerpad_device)

#endif // MAME_BUS_NES_CTRL_POWERPAD_H
