/*
 * apollo_kbd.h
 *
 *  Created on: Dec 27, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 */

#pragma once

#ifndef __APOLLO_KBD_H__
#define __APOLLO_KBD_H__

#include "emu.h"

// BSD-derived systems get very sad when you party with system reserved names.
#ifdef getchar
#undef getchar
#endif

#ifdef putchar
#undef putchar
#endif

#define TX_FIFO_SIZE 128

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_APOLLO_KBD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, APOLLO_KBD, 0) \
	apollo_kbd_device::static_set_interface(*device, _interface);

INPUT_PORTS_EXTERN(apollo_kbd);

//**************************************************************************
// Keyboard READ/WRITE
//**************************************************************************

void apollo_kbd_getchar(device_t *device, UINT8 data);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apollo_kbd_interface

struct apollo_kbd_interface
{
	devcb_write8 apollo_kbd_putchar_cb;
	devcb_read8 apollo_kbd_has_beeper_cb;
	devcb_read8 apollo_kbd_is_german_cb;
};

#define APOLLO_KBD_INTERFACE(name) const struct apollo_kbd_interface (name)

// ======================> apollo_kbd_device

class apollo_kbd_device :	public device_t, public apollo_kbd_interface
{
public:
	// construction/destruction
	apollo_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const apollo_kbd_interface &interface);

	void getchar(UINT8 data);

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	const char *cpu_context() ;

	int keyboard_is_german();

	void set_mode(UINT16 mode);
	void putchar(const UINT8 data);
	void putdata(const UINT8 *data, int data_length);
	void putstring(const char *data);

	int push_scancode( UINT8 code, UINT8 repeat);
	void scan_keyboard();
	void poll_callback();
	static TIMER_CALLBACK( static_poll_callback );

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
		void beeper_callback();
		static TIMER_CALLBACK( static_beeper_callback );

		apollo_kbd_device *m_device; // pointer back to our device
		device_t *m_beeper; // the keyboard beeper device
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

		int m_last_b;  // previous mouse button values
		int m_last_x;  // previous mouse x-axis value
		int m_last_y;  // previous mouse y-axis value
		int m_tx_pending;  // mouse data packet is pending
	};

    /* Transmitter fifo */
	class tx_fifo
	{
	public:
		tx_fifo();
		void start(apollo_kbd_device *device);
		void reset();
		UINT8 getchar();
		void putchar(UINT8 data);
		int putdata(const UINT8 *data, int data_length);
		void flush();

	private:
		void timer_callback();
		static TIMER_CALLBACK( static_timer_callback );

		apollo_kbd_device *m_device; // pointer back to our device

		UINT16 m_baud_rate;

		UINT16 fifo[TX_FIFO_SIZE];
		UINT16 m_read_ptr;
		UINT16 m_write_ptr;
		UINT16 m_char_count;
		UINT16 m_tx_pending;
		emu_timer *m_timer;
	};

	// the keyboard tty
	class keyboard_tty
	{
	public:
		keyboard_tty();
		void start(apollo_kbd_device *device);
		void reset();
		int isConnected();
		int getchar();
		void putchar(UINT8 data);
	private:
		apollo_kbd_device *m_device; // pointer back to our device
		const char *m_tty_name;
		int m_tty_fd; /* File descriptor of keyboard tty */
		int m_connected;
	};

//  const apollo_kbd_interface &m_config;

	beeper	m_beeper;
	mouse	m_mouse;
    tx_fifo m_tx_fifo;
    keyboard_tty  m_keyboard_tty;

	apollo_kbd_device *m_device; // pointer to myself (nasty: used for cpu_context)

	/* Receiver */
	UINT32 m_rx_message;
	UINT16 m_loopback_mode;

	emu_timer* m_timer;
	UINT16 m_mode;
	UINT16 m_delay;         // key press delay after initial press
	UINT16 m_repeat;        // key press repeat rate
	UINT16 m_last_pressed;  // last key pressed, for repeat key handling
	int m_keytime[0x80];    // time until next key press (1 ms)
	UINT8 m_keyon[0x80];    // is 1 if key is pressed

	devcb_resolved_write8 m_putchar;
	devcb_resolved_read8 m_has_beeper;
	devcb_resolved_read8 m_is_german;

	static UINT16 m_code_table[];
};

// device type definition
extern const device_type APOLLO_KBD;

#endif
