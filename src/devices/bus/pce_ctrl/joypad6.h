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

// ======================> pce_joypad6_base_device

class pce_joypad6_base_device : public device_t,
							public device_pce_control_port_interface
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(joypad_mode_changed);

protected:
	// construction/destruction
	pce_joypad6_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

	// button handlers
	void buttonset_update();

	// internal states
	u8 m_counter = 0; // buttonset select, autofire counter (74xx163 QA-QB pin)
	bool m_prev_clr = false; // previous CLR pin state

	// devices
	required_device_array<ls157_device, 3> m_muxer;

	// IO ports
	required_ioport m_joypad_mode;
};


// ======================> pce_avenue_pad_6_device

class pce_avenue_pad_6_device : public pce_joypad6_base_device
{
public:
	// construction/destruction
	pce_avenue_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// construction/destruction
	pce_avenue_pad_6_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

	template<unsigned Buttonset> u8 buttons_r();

	// IO ports
	required_ioport_array<2> m_buttons_io;
	required_ioport m_turbo_io;
};


// ======================> pce_arcade_pad_6_device

class pce_arcade_pad_6_device : public pce_avenue_pad_6_device
{
public:
	// construction/destruction
	pce_arcade_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_AVENUE_PAD_6, pce_avenue_pad_6_device)
DECLARE_DEVICE_TYPE(PCE_ARCADE_PAD_6, pce_arcade_pad_6_device)


#endif // MAME_BUS_PCE_CTRL_JOYPAD6_H
