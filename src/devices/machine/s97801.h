// license:BSD-3-Clause
// copyright-holders:Dave Rand
#ifndef MAME_MACHINE_S97801_H
#define MAME_MACHINE_S97801_H

#pragma once

#include "cpu/mcs51/i8051.h"
#include "machine/s97801_kbd.h"
#include "machine/scn_pci.h"
#include "video/scn2674.h"

#include "screen.h"

// Siemens 97801 terminal (LLE) -- core device.  The serial console for SINIX on the PC-MX2.
// Exposed as a serial peripheral: rxd_w() is the
// host->terminal line, txd_handler() the terminal->host line (the firmware runs the SCN2661 EPCI
// at 38400 7O1 XON/XOFF).  The detached keyboard is the s97801_kbd LLE device on the 8031's
// on-chip UART (600 baud both ways, firmware-timed).
class s97801_device : public device_t
{
public:
	s97801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto txd_handler() { return m_txd_cb.bind(); } // terminal -> host

	// live signals
	void rxd_w(int state);                          // host -> terminal

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// configuration
	void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	// live signals
	void epci_txd_w(int state) { m_txd_cb(state); }
	void kbd_txd_w(int state) { m_kbd_rxd = state; }
	u8 cpu_p3_r();
	void cpu_p3_w(u8 data);
	u8 e00x_status_r(offs_t offset);
	void e00x_status_w(offs_t offset, u8 data);

	required_device<i8031_device> m_cpu;
	required_device<scn2672_device> m_avdc;
	required_device<scn2661b_device> m_epci;
	required_device<s97801_kbd_device> m_kbd;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_chargen;
	devcb_write_line m_txd_cb;

	u8 m_kbd_rxd; // keyboard -> terminal serial line, sampled by the 8031 UART on P3.0
};

DECLARE_DEVICE_TYPE(SIEMENS_97801, s97801_device)

#endif // MAME_MACHINE_S97801_H
