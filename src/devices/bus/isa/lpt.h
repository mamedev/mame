// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#ifndef MAME_BUS_ISA_LPT_H
#define MAME_BUS_ISA_LPT_H

#pragma once

#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_lpt_device

class isa8_lpt_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool is_primary() { return m_is_primary; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void pc_cpu_line(int state);

	// internal state
	bool m_is_primary;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_LPT, isa8_lpt_device)

#endif // MAME_BUS_ISA_LPT_H
