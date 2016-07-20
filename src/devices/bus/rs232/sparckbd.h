// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_RS232_SPARCKBD_H
#define MAME_DEVICES_RS232_SPARCKBD_H

#pragma once

#include "rs232.h"
#include "sound/beep.h"


class sparc_keyboard_device : public device_t, public device_serial_interface, public device_rs232_port_interface
{
public:
	sparc_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);
	sparc_keyboard_device(machine_config const &mconfig, device_type type, char const *name, char const *tag, device_t *owner, UINT32 clock, char const *shortname, char const *source);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

protected:
	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	// device_serial_interface uses 10'000 range
	enum {
		SCAN_TIMER_ID = 20'000,
		CLICK_TIMER_ID
	};

	// TODO: ensure these don't clash with diagnostic LEDs on host computer
	enum : int {
		LED_NUM = 0,
		LED_COMPOSE,
		LED_SCROLL,
		LED_CAPS
	};

	enum : UINT8 {
		BEEPER_BELL = 0x01U,
		BEEPER_CLICK = 0x02U
	};

	enum : UINT8 {
		RX_IDLE,
		RX_LED
	};

	void scan_row();
	void send_byte(UINT8 code);

	emu_timer                       *m_scan_timer;
	emu_timer                       *m_click_timer;
	required_ioport                 m_dips;
	required_ioport                 m_key_inputs[8];
	required_device<beep_device>    m_beeper;

	UINT16  m_current_keys[8];
	UINT8   m_next_row;

	UINT8   m_fifo[16];
	UINT8   m_head, m_tail;
	UINT8   m_empty;

	UINT8   m_rx_state;

	UINT8   m_keyclick;
	UINT8   m_beeper_state;
};

extern const device_type SPARC_KEYBOARD;

#endif // MAME_DEVICES_RS232_SPARCKBD_H
