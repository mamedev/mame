// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Yonezawa / Party Room 21 Partytap Controller

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_PARTYTAP_H
#define MAME_BUS_NES_CTRL_PARTYTAP_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_partytap_device

class nes_partytap_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_partytap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_inputs;
	u8 m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_PARTYTAP, nes_partytap_device)

#endif // MAME_BUS_NES_CTRL_PARTYTAP_H
