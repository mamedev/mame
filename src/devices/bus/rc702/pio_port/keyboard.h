// license:BSD-3-Clause
// copyright-holders:Thorbjørn Ravn Andersen
/***************************************************************************

    RC702 keyboard -- Z80 PIO port slot card

    Encapsulates the RC702's keyboard input behaviour: a generic keyboard
    widget supplies typed bytes; on each keystroke the byte is latched
    and a strobe pulse is asserted into the PIO chip.  This is a
    refactor of the inline `kbd_put` / `kbd_r` plumbing that used to
    live in rc702.cpp.

    The real RC702 keyboard is a smart peripheral with its own 8048 +
    2758 (both undumped); this card uses MAME's generic_keyboard_device
    as the user-facing typing widget, which has been the convention in
    rc702.cpp since the 2016 skeleton.

***************************************************************************/

#ifndef MAME_BUS_RC702_PIO_PORT_KEYBOARD_H
#define MAME_BUS_RC702_PIO_PORT_KEYBOARD_H

#pragma once

#include "pio_port.h"

#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc702_pio_keyboard_device :
	public device_t,
	public device_rc702_pio_port_interface
{
public:
	rc702_pio_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_rc702_pio_port_interface
	virtual uint8_t read() override { return m_kbd_data; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void kbd_put(uint8_t data);

	required_device<generic_keyboard_device> m_keyboard;
	uint8_t m_kbd_data;
};

DECLARE_DEVICE_TYPE(RC702_PIO_KEYBOARD, rc702_pio_keyboard_device)

#endif // MAME_BUS_RC702_PIO_PORT_KEYBOARD_H
