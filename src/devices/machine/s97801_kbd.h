// license:BSD-3-Clause
// copyright-holders:Dave Rand
#ifndef MAME_MACHINE_S97801_KBD_H
#define MAME_MACHINE_S97801_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"

// Siemens 97801 detached keyboard (S26361-K142 family) -- LLE.  A Philips MAB 8035HL runs the
// K111 EPROM's MCS-48 firmware, scanning a 16x8 matrix plus a modifier column and speaking a
// firmware-timed 600-baud async serial link to the terminal: rxd_w() is the terminal->keyboard
// line, txd_handler() the keyboard->terminal line.  The bell lives here, not in the terminal.
class s97801_kbd_device : public device_t
{
public:
	s97801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto txd_handler() { return m_txd_cb.bind(); } // keyboard -> terminal

	// live signals
	void rxd_w(int state);                         // terminal -> keyboard

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void prg_map(address_map &map) ATTR_COLD;

	// live signals
	u8 bus_r();
	void bus_w(u8 data);
	void p1_w(u8 data);
	void p2_w(u8 data);
	int t0_r() { return m_rxd; }
	int t1_r() { return !m_rxd; } // complement side of the RX pair
	TIMER_CALLBACK_MEMBER(bell_off);

	required_device<i8035_device> m_mcu;
	required_device<beep_device> m_beep;
	required_ioport_array<16> m_cols;
	required_ioport m_mod;
	devcb_write_line m_txd_cb;
	output_finder<8> m_lamps;    // host-controlled indicator lamps (commands $01-$0F)
	output_finder<> m_led_caps;  // CAPS key LED (P2.7)
	output_finder<> m_led_lock;  // LOCK key LED (P2.6)

	emu_timer *m_bell_timer;
	u8 m_p1;
	u8 m_p2;
	u8 m_rxd;
};

DECLARE_DEVICE_TYPE(SIEMENS_97801_KBD, s97801_kbd_device)

#endif // MAME_MACHINE_S97801_KBD_H
