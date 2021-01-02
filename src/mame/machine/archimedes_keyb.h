// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Sandro Ronco
/**********************************************************************

    Acorn Archimedes keyboard HLE

*********************************************************************/

#ifndef MAME_MACHINE_ARCHIMEDES_KEYB_H
#define MAME_MACHINE_ARCHIMEDES_KEYB_H

#pragma once

#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> archimedes_keyboard_device

class archimedes_keyboard_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	archimedes_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto kout() { return m_kout.bind(); }

	DECLARE_WRITE_LINE_MEMBER(kin_w) { rx_w(state); }
	DECLARE_INPUT_CHANGED_MEMBER(update_mouse_input);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	void exec_command(uint8_t command);
	void scan_keyb();

	devcb_write_line m_kout;
	required_ioport_array<8> m_keyboard;
	required_ioport_array<2> m_mouse;
	output_finder<3> m_leds;

	emu_timer * m_scan_timer;
	bool        m_keyb_enable;
	bool        m_mouse_enable;
	int16_t     m_mouse_x;
	int16_t     m_mouse_y;
	uint16_t    m_data;
	bool        m_states[0x80];
};


// device type definition
DECLARE_DEVICE_TYPE(ARCHIMEDES_KEYBOARD, archimedes_keyboard_device)


#endif // MAME_MACHINE_ARCHIMEDES_KEYB_H
