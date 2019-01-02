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
	nes_powerpad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_bit34() override;
	virtual void write(uint8_t data) override;

private:
	required_ioport m_ipt1;
	required_ioport m_ipt2;
	uint32_t m_latch[2];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_POWERPAD, nes_powerpad_device)

#endif // MAME_BUS_NES_CTRL_POWERPAD_H
