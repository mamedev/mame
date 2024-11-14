// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Bandai Hyper Shot Lightgun

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_BANDAIHS
#define MAME_BUS_NES_CTRL_BANDAIHS

#pragma once

#include "ctrl.h"
#include "zapper_sensor.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_bandaihs_device

class nes_bandaihs_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_bandaihs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<nes_zapper_sensor_device> m_sensor;
	required_ioport m_lightx;
	required_ioport m_lighty;
	required_ioport m_trigger;
	required_ioport m_joypad;
	u8 m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BANDAIHS, nes_bandaihs_device)

#endif // MAME_BUS_NES_CTRL_BANDAIHS
