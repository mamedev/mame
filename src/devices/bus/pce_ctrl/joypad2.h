// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 2 Button Joypad emulation

**********************************************************************/

#ifndef MAME_BUS_PCE_CTRL_JOYPAD2_H
#define MAME_BUS_PCE_CTRL_JOYPAD2_H

#pragma once


#include "machine/74157.h"
#include "pcectrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_joypad2_device

class pce_joypad2_device : public device_t,
							public device_pce_control_port_interface
{
public:
	// construction/destruction
	pce_joypad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// construction/destruction
	pce_joypad2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

	// devices
	required_device<ls157_device> m_muxer;
};

// ======================> pce_joypad2_turbo_device

class pce_joypad2_turbo_device : public pce_joypad2_device
{
public:
	// construction/destruction
	pce_joypad2_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pce_control_port_interface overrides
	virtual void clr_w(int state) override;

private:
	u8 buttons_r();

	// internal states
	u8 m_counter = 0; // Turbo rate counter, connected on 74xx163 QB and QC.
	bool m_prev_clr = false; // previous CLR pin state

	// IO ports
	required_ioport m_buttons_io;
	required_ioport m_turbo_io;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_JOYPAD2,       pce_joypad2_device)
DECLARE_DEVICE_TYPE(PCE_JOYPAD2_TURBO, pce_joypad2_turbo_device)


#endif // MAME_BUS_PCE_CTRL_JOYPAD2_H
