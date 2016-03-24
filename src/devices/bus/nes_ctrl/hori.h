// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Hori Twin (and 4P?) adapters

**********************************************************************/

#pragma once

#ifndef __NES_HORI__
#define __NES_HORI__


#include "emu.h"
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
	nes_horitwin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override {}

	virtual UINT8 read_exp(offs_t offset) override;
	virtual void write(UINT8 data) override;

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
	nes_hori4p_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override {}

	virtual UINT8 read_exp(offs_t offset) override;
	virtual void write(UINT8 data) override;

private:
	required_device<nes_control_port_device> m_port1;
	required_device<nes_control_port_device> m_port2;
	required_device<nes_control_port_device> m_port3;
	required_device<nes_control_port_device> m_port4;
	required_ioport m_cfg;
};


// device type definition
extern const device_type NES_HORITWIN;
extern const device_type NES_HORI4P;


#endif
