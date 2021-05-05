// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 6 Button Joypad emulation

**********************************************************************/

#ifndef MAME_BUS_PCE_CTRL_JOYPAD6_H
#define MAME_BUS_PCE_CTRL_JOYPAD6_H

#pragma once


#include "machine/74157.h"
#include "pcectrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_joypad6_device

class pce_joypad6_device : public device_t,
							public device_pce_control_port_interface
{
public:
	// construction/destruction
	pce_joypad6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_INPUT_CHANGED_MEMBER(joypad_mode_changed);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void clk_w(int state) override;
	virtual void rst_w(int state) override;

	// button handlers
	void buttonset_update();

	// internal states
	u8 m_button_sel = 0; // buttonset select, autofire counter (74xx163 Q1-Q3 pin)
	bool m_prev_rst = false; // previous Reset pin state

	// devices
	required_device_array<ls157_device, 3> m_muxer;

	// IO ports
	required_ioport m_joypad_mode;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_JOYPAD6, pce_joypad6_device)


#endif // MAME_BUS_PCE_CTRL_JOYPAD6_H
