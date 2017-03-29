// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System Four Score Adapter

**********************************************************************/

#pragma once

#ifndef __NES_FOURSCORE__
#define __NES_FOURSCORE__


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_4score_device

class nes_4score_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_4score_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_bit0() override;

protected:
	uint32_t m_latch;
};

// ======================> nes_4score_p1p3_device

class nes_4score_p1p3_device : public nes_4score_device
{
public:
	// construction/destruction
	nes_4score_p1p3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void write(uint8_t data) override;

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

	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void write(uint8_t data) override;

private:
	required_ioport m_joypad2;
	required_ioport m_joypad4;
};


// device type definition
extern const device_type NES_4SCORE_P1P3;
extern const device_type NES_4SCORE_P2P4;


#endif
