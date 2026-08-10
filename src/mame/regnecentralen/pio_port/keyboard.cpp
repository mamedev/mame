// license:BSD-3-Clause
// copyright-holders:Thorbjørn Ravn Andersen
/***************************************************************************

    RC702 keyboard -- Z80 PIO port slot card

    The real RC702 keyboard is a smart peripheral with its own 8048 + 2758
    (both undumped); this card uses MAME's generic_keyboard_device as the
    user-facing typing widget.  Each keystroke latches the byte and pulses
    STB into the Z80 PIO chip.

***************************************************************************/

#include "emu.h"
#include "keyboard.h"

#include "machine/keyboard.h"


namespace {

class rc702_pio_keyboard_device :
	public device_t,
	public device_rc702_pio_port_interface
{
public:
	rc702_pio_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(RC702_PIO_KEYBOARD, device_rc702_pio_port_interface, rc702_pio_keyboard_device, "rc702_pio_keyboard", "RC702 Keyboard")
