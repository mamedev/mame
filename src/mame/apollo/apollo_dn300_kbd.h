// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, Chris Toshok, Vladimir Vukicevic
/*
 * apollo_dn300_kbd.h - the Apollo DN300/DN320 keyboard and mouse
 */

#ifndef MAME_APOLLO_APOLLO_DN300_KBD_H
#define MAME_APOLLO_APOLLO_DN300_KBD_H

#pragma once

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apollo_dn300_kbd_device

class apollo_dn300_kbd_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	apollo_dn300_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_cb() { return m_tx_w.bind(); }

private:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// serial overrides
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	TIMER_CALLBACK_MEMBER( kbd_scan_timer );

	int push_scancode(uint8_t code);
	void scan_keyboard();
	void read_mouse();

	static const int XMIT_RING_SIZE = 64;

	required_ioport_array<4> m_io_keyboard;
	required_ioport_array<3> m_io_mouse;

	devcb_write_line m_tx_w;

	uint8_t m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	bool m_tx_busy;

	void xmit_char(uint8_t data);

	// the mouse
	int m_mouse_last_b;      // previous mouse button values
	int m_mouse_last_x;      // previous mouse x-axis value
	int m_mouse_last_y;      // previous mouse y-axis value
	int m_mouse_tx_pending;  // mouse data packet is pending

	emu_timer* m_timer;
	uint8_t m_keyon[0x80];    // is 1 if key is pressed

	// the keyboard's code table; only the compatibility mode codes are sent
	// (the unshifted, shifted, control and caps lock columns), the others
	// document the keyboard
	struct code_entry { uint16_t down, up, unshifted, shifted, control, caps_lock, up_trans, auto_repeat; };
	static code_entry const s_code_table[];
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_DN300_KBD, apollo_dn300_kbd_device)

#endif // MAME_APOLLO_APOLLO_DN300_KBD_H
