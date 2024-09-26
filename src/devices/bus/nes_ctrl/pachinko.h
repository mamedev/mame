// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Coconuts Japan CJPC-102 Pachinko Controller

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_PACHINKO_H
#define MAME_BUS_NES_CTRL_PACHINKO_H

#pragma once

#include "joypad.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_pachinko_device

class nes_pachinko_device : public nes_fcpadexp_device
{
public:
	// construction/destruction
	nes_pachinko_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_latch() override;

private:
	required_ioport m_trigger;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_PACHINKO, nes_pachinko_device)

#endif // MAME_BUS_NES_CTRL_PACHINKO_H
