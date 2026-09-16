// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Capcom Mahjong Controller

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_MJPANEL_H
#define MAME_BUS_NES_CTRL_MJPANEL_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_mjpanel_device

class nes_mjpanel_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_mjpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void set_latch();

	required_ioport_array<4> m_panel;
	u8 m_latch;
	u8 m_row;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_MJPANEL, nes_mjpanel_device)

#endif // MAME_BUS_NES_CTRL_MJPANEL_H
