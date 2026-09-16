// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System Four Score Adapter

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_4SCORE_H
#define MAME_BUS_NES_CTRL_4SCORE_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_4score_device

class nes_4score_device : public device_t,
							public device_nes_control_port_interface
{
public:
	virtual uint8_t read_bit0() override;

protected:
	// construction/destruction
	nes_4score_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint32_t m_latch;
};

// ======================> nes_4score_p1p3_device

class nes_4score_p1p3_device : public nes_4score_device
{
public:
	// construction/destruction
	nes_4score_p1p3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(uint8_t data) override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joypad1;
	required_ioport m_joypad3;
};

// ======================> nes_4score_p2p4_device

class nes_4score_p2p4_device : public nes_4score_device
{
public:
	// construction/destruction
	nes_4score_p2p4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(uint8_t data) override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joypad2;
	required_ioport m_joypad4;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_4SCORE_P1P3, nes_4score_p1p3_device)
DECLARE_DEVICE_TYPE(NES_4SCORE_P2P4, nes_4score_p2p4_device)

#endif // MAME_BUS_NES_CTRL_4SCORE_H
