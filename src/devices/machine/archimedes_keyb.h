// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Sandro Ronco
/**********************************************************************

    Acorn Archimedes keyboard

*********************************************************************/

#ifndef MAME_MACHINE_ARCHIMEDES_KEYB_H
#define MAME_MACHINE_ARCHIMEDES_KEYB_H

#pragma once

#include "cpu/mcs51/mcs51.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> archimedes_keyboard_device

class archimedes_keyboard_device : public device_t
{
public:
	// construction/destruction
	archimedes_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto kout() { return m_kout.bind(); }

	void kin_w(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_mouse);

private:
	uint8_t mouse_r();
	void leds_w(uint8_t data);

	required_device<mcs51_cpu_device> m_mcu;
	devcb_write_line m_kout;
	required_ioport_array<16> m_keyboard;
	required_ioport_array<3> m_mouse;
	output_finder<3> m_leds;

	emu_timer *m_mouse_timer;
	uint8_t    m_mouse_xphase;
	uint8_t    m_mouse_xdir;
	uint8_t    m_mouse_xref;
	uint8_t    m_mouse_yphase;
	uint8_t    m_mouse_ydir;
	uint8_t    m_mouse_yref;
	int16_t    m_mouse_x;
	int16_t    m_mouse_y;
	uint8_t    m_mux;
	bool       m_kin;
};


// device type definition
DECLARE_DEVICE_TYPE(ARCHIMEDES_KEYBOARD, archimedes_keyboard_device)


#endif // MAME_MACHINE_ARCHIMEDES_KEYB_H
