// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Keyboard Interface

**********************************************************************/

#ifndef MAME_BUS_TANBUS_KEYBOARD_H
#define MAME_BUS_TANBUS_KEYBOARD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_microtan_kbd_interface;

// ======================> microtan_kbd_slot_device

class microtan_kbd_slot_device : public device_t, public device_single_card_slot_interface<device_microtan_kbd_interface>
{
public:
	// construction/destruction
	template <typename T>
	microtan_kbd_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: microtan_kbd_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	microtan_kbd_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// callbacks
	auto strobe_handler() { return m_strobe_handler.bind(); }
	auto reset_handler() { return m_reset_handler.bind(); }

	virtual uint8_t read();
	virtual void write(uint8_t data);

	void strobe_w(int state) { m_strobe_handler(state); }
	void reset_w(int state) { m_reset_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	device_microtan_kbd_interface *m_kbd;

	devcb_write_line m_strobe_handler;
	devcb_write_line m_reset_handler;
};


// ======================> device_microtan_kbd_interface

class device_microtan_kbd_interface : public device_interface
{
public:
	virtual uint8_t read() { return 0xff; }
	virtual void write(uint8_t data) { }

protected:
	device_microtan_kbd_interface(const machine_config &mconfig, device_t &device);

	microtan_kbd_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(MICROTAN_KBD_SLOT, microtan_kbd_slot_device)

void microtan_kbd_devices(device_slot_interface &device);


#endif // MAME_BUS_TANBUS_KEYBOARD_H
