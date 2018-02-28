// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT100 keyboard emulation

***************************************************************************/

#ifndef MAME_MACHINE_VT100_KBD_H
#define MAME_MACHINE_VT100_KBD_H

#pragma once

#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"


//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VT100_KEYBOARD_INT_CALLBACK(_devcb) \
	devcb = &downcast<vt100_keyboard_device &>(*device).set_int_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vt100_keyboard_device

class vt100_keyboard_device : public device_t
{
public:
	// construction/destruction
	vt100_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <class Object> devcb_base &set_int_callback(Object &&cb) { return m_int_cb.set_callback(std::forward<Object>(cb)); }

	// accessors (for now)
	void control_w(u8 data);
	u8 key_code_r();

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	// internal helpers
	static u8 bit_sel(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(scan_callback);

	devcb_write_line m_int_cb;

	required_device<beep_device> m_speaker;
	required_ioport_array<16> m_key_row;
	bool m_key_scan;
	u8 m_key_code;
};

// device type definition
DECLARE_DEVICE_TYPE(VT100_KEYBOARD, vt100_keyboard_device)

#endif // MAME_MACHINE_VT100_KBD_H
