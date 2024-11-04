// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Keyboard (MT009)

*********************************************************************/

#ifndef MAME_BUS_TANBUS_KEYBOARD_MT009_H
#define MAME_BUS_TANBUS_KEYBOARD_MT009_H

#pragma once

#include "keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class microtan_kbd_mt009 : public device_t, public device_microtan_kbd_interface
{
public:
	// construction/destruction
	microtan_kbd_mt009(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	TIMER_CALLBACK_MEMBER(kbd_scan);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read() override;

private:
	required_memory_region m_rom;
	required_ioport_array<9> m_keyboard;

	emu_timer *m_kbd_scan_timer = nullptr;

	uint8_t m_kbd_ascii = 0;
	uint8_t m_keyrows[10]{};
	int m_lastrow = 0;
	int m_mask = 0;
	int m_key = 0;
	int m_repeat = 0;
	int m_repeater = 0;

	void store_key(int key);
};


// device type definition
DECLARE_DEVICE_TYPE(MICROTAN_KBD_MT009, microtan_kbd_mt009)


#endif // MAME_BUS_TANBUS_KEYBOARD_MT009_H
