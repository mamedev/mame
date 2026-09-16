// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System -
    Arkanoid Paddle input device

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ARKPADDLE_H
#define MAME_BUS_NES_CTRL_ARKPADDLE_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_vaus_device

class nes_vaus_device : public device_t, public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit34() override;
	virtual void write(u8 data) override;

protected:
	nes_vaus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport m_paddle;
	required_ioport m_button;
	u16 m_latch;
};


// ======================> nes_vausfc_device

class nes_vausfc_device : public nes_vaus_device
{
public:
	// construction/destruction
	nes_vausfc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit34() override { return 0; }
	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<nes_control_port_device> m_daisychain;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ARKPADDLE,    nes_vaus_device)
DECLARE_DEVICE_TYPE(NES_ARKPADDLE_FC, nes_vausfc_device)

#endif // MAME_BUS_NES_CTRL_ARKPADDLE_H
