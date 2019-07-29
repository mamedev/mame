// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Hori Twin (and 4P?) adapters

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_HORI_H
#define MAME_BUS_NES_CTRL_HORI_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_horitwin_device

class nes_horitwin_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_horitwin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

private:
	required_device<nes_control_port_device> m_port1;
	required_device<nes_control_port_device> m_port2;
};

// ======================> nes_hori4p_device

class nes_hori4p_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_hori4p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

private:
	required_device<nes_control_port_device> m_port1;
	required_device<nes_control_port_device> m_port2;
	required_device<nes_control_port_device> m_port3;
	required_device<nes_control_port_device> m_port4;
	required_ioport m_cfg;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_HORITWIN, nes_horitwin_device)
DECLARE_DEVICE_TYPE(NES_HORI4P,   nes_hori4p_device)


#endif // MAME_BUS_NES_CTRL_HORI_H
