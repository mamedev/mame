// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES Multitap Adapter

**********************************************************************/

#ifndef MAME_BUS_SNES_CTRL_MULTITAP_H
#define MAME_BUS_SNES_CTRL_MULTITAP_H

#pragma once


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_multitap_device

class snes_multitap_device : public device_t,
							public device_snes_control_port_interface
{
public:
	// construction/destruction
	snes_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_snes_control_port_interface overrides
	virtual uint8_t read_pin4() override;
	virtual uint8_t read_pin5() override;
	virtual void write_strobe(uint8_t data) override;
	virtual void write_pin6(uint8_t data) override;
	virtual void port_poll() override;

private:
	required_device<snes_control_port_device> m_port1;
	required_device<snes_control_port_device> m_port2;
	required_device<snes_control_port_device> m_port3;
	required_device<snes_control_port_device> m_port4;
	required_ioport m_cfg;
	int m_select;
};


// device type definition
DECLARE_DEVICE_TYPE(SNES_MULTITAP, snes_multitap_device)

#endif // MAME_BUS_SNES_CTRL_MULTITAP_H
