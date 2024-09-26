// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Dial controller emulation

**********************************************************************/

#ifndef MAME_BUS_NEOGEO_CTRL_DIAL_H
#define MAME_BUS_NEOGEO_CTRL_DIAL_H

#pragma once


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_dial_device

class neogeo_dial_device : public device_t, public device_neogeo_ctrl_edge_interface
{
public:
	// construction/destruction
	neogeo_dial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_neogeo_control_port_interface overrides
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void write_ctrlsel(uint8_t data) override;
	virtual uint8_t read_start_sel() override;

private:
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_dial1;
	required_ioport m_dial2;
	required_ioport m_ss;
	uint8_t m_ctrl_sel;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_DIAL, neogeo_dial_device)

#endif // MAME_BUS_NEOGEO_CTRL_DIAL_H
