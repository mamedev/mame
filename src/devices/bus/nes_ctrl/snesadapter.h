// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer & Entertainment System SNES controller port adapter

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_SNESADAPTER_H
#define MAME_BUS_NES_CTRL_SNESADAPTER_H

#pragma once

#include "ctrl.h"
#include "bus/snes_ctrl/ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_snesadapter_device

class nes_snesadapter_device : public device_t, public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_snesadapter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit0() override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<snes_control_port_device> m_snesctrl;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SNESADAPTER, nes_snesadapter_device)

#endif // MAME_BUS_NES_CTRL_SNESADAPTER_H
