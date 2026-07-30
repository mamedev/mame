// license:BSD-3-Clause
// copyright-holders:Thorbjørn Ravn Andersen
/***************************************************************************

    RC702 keyboard -- Z80 PIO port slot card

***************************************************************************/

#include "emu.h"
#include "keyboard.h"


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(RC702_PIO_KEYBOARD, rc702_pio_keyboard_device, "rc702_pio_keyboard", "RC702 Keyboard")


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

rc702_pio_keyboard_device::rc702_pio_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, RC702_PIO_KEYBOARD, tag, owner, clock),
	device_rc702_pio_port_interface(mconfig, *this),
	m_keyboard(*this, "kbd"),
	m_kbd_data(0)
{
}

void rc702_pio_keyboard_device::device_start()
{
	save_item(NAME(m_kbd_data));
}

void rc702_pio_keyboard_device::device_add_mconfig(machine_config &config)
{
	GENERIC_KEYBOARD(config, m_keyboard, 0);
	m_keyboard->set_keyboard_callback(FUNC(rc702_pio_keyboard_device::kbd_put));
}

void rc702_pio_keyboard_device::kbd_put(uint8_t data)
{
	m_kbd_data = data;
	// Pulse STB low->high; PIO latches data on falling edge.
	m_slot->strobe_w(0);
	m_slot->strobe_w(1);
}
