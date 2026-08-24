// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xerox 820/820-II ASCII keyboard emulation

*********************************************************************/

#ifndef MAME_XEROX_X820KB_H
#define MAME_XEROX_X820KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xerox820_keyboard_device

// Abstract base for the keyboards a Xerox 820-family machine ships with (the
// standard 8748 ASCII keyboard or the position-encoded Low Profile Keyboard):
// each presents a latched byte plus a KBSTB strobe to the system PIO.  The
// machine configuration fixes which one is fitted (the line never offered
// mix-and-match keyboards), so the driver holds a single required device of
// this type.

class xerox820_keyboard_device : public device_t
{
public:
	auto kbstb_wr_callback() { return m_kbstb_cb.bind(); }

	virtual uint8_t read() = 0; // the latched key byte presented to the system PIO

protected:
	xerox820_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	devcb_write_line m_kbstb_cb;
};


// ======================> xerox_820_keyboard_device

class xerox_820_keyboard_device : public xerox820_keyboard_device
{
public:
	// construction/destruction
	xerox_820_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t read() override { return m_bus; }

protected:
	xerox_820_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t m_bus;

private:
	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	uint8_t kb_p2_r();
	int kb_t0_r();
	int kb_t1_r();
	void kb_bus_w(uint8_t data);

	required_device<i8048_device> m_maincpu;
	required_ioport_array<16> m_y;

	uint8_t m_p1;
};


// ======================> xerox_820ii_keyboard_device

// Same hardware and ASCII key encoding as the 820 keyboard, but emits the
// power-on "keyboard available" announce the 820-II monitor waits for (the
// original 820 has no announce).

class xerox_820ii_keyboard_device : public xerox_820_keyboard_device
{
public:
	// construction/destruction
	xerox_820ii_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

private:
	TIMER_CALLBACK_MEMBER(announce);

	emu_timer *m_announce_timer;
};


// device type definitions
DECLARE_DEVICE_TYPE(XEROX_820_KEYBOARD, xerox_820_keyboard_device)
DECLARE_DEVICE_TYPE(XEROX_820II_KEYBOARD, xerox_820ii_keyboard_device)


#endif // MAME_XEROX_X820KB_H
