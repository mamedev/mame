// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn Keyboard - Part No. 200,013

****************************************************************************/

#ifndef MAME_ACORN_ACRNSYS_KBD_H
#define MAME_ACORN_ACRNSYS_KBD_H

#pragma once

#include "machine/ay34592.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acrnsys_keyboard_device

class acrnsys_keyboard_device : public device_t
{
public:
	// construction/destruction
	acrnsys_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto rst_handler() { return m_rst_handler.bind(); }
	auto strobe_handler() { return m_strobe_handler.bind(); }
	auto blank_handler() { return m_blank_handler.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(blank_changed);
	DECLARE_INPUT_CHANGED_MEMBER(caps_changed);
	DECLARE_INPUT_CHANGED_MEMBER(repeat_changed);
	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

	uint8_t data_r();
	int strobe_r();
	int blank_r();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<ay34592_device> m_ay34592;

	devcb_write_line m_rst_handler;
	devcb_write_line m_strobe_handler;
	devcb_write_line m_blank_handler;

	emu_timer *m_repeat_timer;

	output_finder<> m_caps_led;
	output_finder<> m_shift_lock_led;
	output_finder<> m_power_led;

	TIMER_CALLBACK_MEMBER(repeat);

	void update_strobe(int state);

	bool m_strobe;
	bool m_blank;
	bool m_capslock;
};

// device type definitions
DECLARE_DEVICE_TYPE(ACRNSYS_KEYBOARD, acrnsys_keyboard_device)

#endif // MAME_ACORN_ACRNSYS_KBD_H
