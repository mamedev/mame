// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Dial controller emulation

**********************************************************************/

#pragma once

#ifndef __NEOGEO_DIAL__
#define __NEOGEO_DIAL__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_dial_device

class neogeo_dial_device : public device_t,
						public device_neogeo_ctrl_edge_interface
{
public:
	// construction/destruction
	neogeo_dial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_neogeo_control_port_interface overrides
	virtual uint8_t in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t in1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ctrlsel(uint8_t data) override;

private:
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_dial1;
	required_ioport m_dial2;
	uint8_t m_ctrl_sel;
};



// device type definition
extern const device_type NEOGEO_DIAL;


#endif
