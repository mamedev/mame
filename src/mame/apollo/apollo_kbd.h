// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer
/*
 * apollo_kbd.h
 *
 *  Created on: Dec 27, 2010
 *      Author: Hans Ostermeyer
 *
 */

#ifndef MAME_APOLLO_APOLLO_KBD_H
#define MAME_APOLLO_APOLLO_KBD_H

#pragma once

#include "sound/beep.h"

#include "diserial.h"


// BSD-derived systems get very sad when you party with system reserved names.
#ifdef getchar
#undef getchar
#endif

#ifdef putchar
#undef putchar
#endif


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apollo_kbd_device

class apollo_kbd_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	apollo_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_cb() { return m_tx_w.bind(); }
	auto german_cb() { return m_german_r.bind(); }

private:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void input_callback(uint8_t state);

	TIMER_CALLBACK_MEMBER( kbd_scan_timer );

	std::string cpu_context() const;
	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const;

	void kgetchar(uint8_t data);

	bool keyboard_is_german();

	void set_mode(uint16_t mode);
	void putdata(const uint8_t *data, int data_length);
	void putstring(const char *data);

	int push_scancode( uint8_t code, uint8_t repeat);
	void scan_keyboard();

	// the keyboard beeper
	class beeper
	{
	public:
		beeper();
		void start(apollo_kbd_device *device);
		void reset();
		void off();
		void on();
	private:
		int keyboard_has_beeper();
		TIMER_CALLBACK_MEMBER(beeper_callback);

		apollo_kbd_device *m_device; // pointer back to our device
		beep_device *m_beeper; // the keyboard beeper device
		emu_timer * m_timer; // timer to clock data in
	};

	// the keyboard mouse
	class mouse
	{
	public:
		mouse();
		void start(apollo_kbd_device *device);
		void reset();
		void read_mouse();
	private:
		apollo_kbd_device *m_device; // pointer back to our device

		int m_last_b = 0;  // previous mouse button values
		int m_last_x = 0;  // previous mouse x-axis value
		int m_last_y = 0;  // previous mouse y-axis value
		int m_tx_pending = 0;  // mouse data packet is pending
	};

	static const int XMIT_RING_SIZE = 64;

	required_device<beep_device> m_beep;
	required_ioport_array<4> m_io_keyboard;
	required_ioport_array<3> m_io_mouse;

	devcb_write_line m_tx_w;
	devcb_read_line m_german_r;

	uint8_t m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	bool m_tx_busy;

	void xmit_char(uint8_t data);

	beeper  m_beeper;
	mouse   m_mouse;

	apollo_kbd_device *m_device; // pointer to myself (nasty: used for cpu_context)

	/* Receiver */
	uint32_t m_rx_message;
	uint16_t m_loopback_mode;

	emu_timer* m_timer;
	uint16_t m_mode;
	uint16_t m_delay;         // key press delay after initial press
	uint16_t m_repeat;        // key press repeat rate
	uint16_t m_last_pressed;  // last key pressed, for repeat key handling
	uint16_t m_numlock_state; // current num lock state
	int m_keytime[0x80];    // time until next key press (1 ms)
	uint8_t m_keyon[0x80];    // is 1 if key is pressed

	struct code_entry { uint16_t down, up, unshifted, shifted, control, caps_lock, up_trans, auto_repeat; };
	static code_entry const s_code_table[];
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_KBD, apollo_kbd_device)

#endif // MAME_APOLLO_APOLLO_KBD_H
