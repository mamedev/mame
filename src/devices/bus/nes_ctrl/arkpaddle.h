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
	nes_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	nes_vaus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_bit34() override;
	virtual void write(uint8_t data) override;

	required_ioport m_paddle;
	required_ioport m_button;
	uint8_t m_start_conv;
	uint32_t m_latch;
};


// ======================> nes_vaus_device

class nes_vausfc_device : public nes_vaus_device
{
public:
	// construction/destruction
	nes_vausfc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint8_t read_bit34() override { return 0; }
	virtual uint8_t read_exp(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ARKPADDLE,    nes_vaus_device)
DECLARE_DEVICE_TYPE(NES_ARKPADDLE_FC, nes_vausfc_device)

#endif // MAME_BUS_NES_CTRL_ARKPADDLE_H
