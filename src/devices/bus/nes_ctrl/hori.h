// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Hori Twin and 4 Players adapters

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
	nes_horitwin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<nes_control_port_device, 2> m_subexp;
};


// ======================> nes_hori4p_device

class nes_hori4p_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_hori4p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void reset_regs();

	required_device_array<nes_control_port_device, 4> m_subexp;
	required_ioport m_cfg;
	u8 m_count[2];
	u8 m_sig[2];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_HORITWIN, nes_horitwin_device)
DECLARE_DEVICE_TYPE(NES_HORI4P,   nes_hori4p_device)

#endif // MAME_BUS_NES_CTRL_HORI_H
