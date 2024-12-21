// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Zenith Z-29 keyboard port

***************************************************************************/

#ifndef MAME_BUS_Z29_KBD_KEYBOARD_H
#define MAME_BUS_Z29_KBD_KEYBOARD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_z29_keyboard_interface;

// ======================> z29_keyboard_port_device

class z29_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_z29_keyboard_interface>
{
	friend class device_z29_keyboard_interface;

public:
	// construction/destruction
	z29_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	z29_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: z29_keyboard_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// callback configuration
	auto keyin_callback() { return m_keyin_callback.bind(); }
	auto reset_callback() { return m_reset_callback.bind(); }

	// line handler
	void keyout_w(int state);

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	// called from keyboard
	void transmit_data(bool state) { m_keyin_callback(state); }
	void reset_from_keyboard(bool state) { m_reset_callback(state); }

private:
	// line callbacks
	devcb_write_line m_keyin_callback;
	devcb_write_line m_reset_callback;

	// selected keyboard
	device_z29_keyboard_interface *m_kbd;
};

// ======================> device_z29_keyboard_interface

class device_z29_keyboard_interface : public device_interface
{
	friend class z29_keyboard_port_device;

protected:
	// construction/destruction
	device_z29_keyboard_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_z29_keyboard_interface();

	void transmit_data(bool state) { m_port->transmit_data(state); }
	void reset_from_keyboard(bool state) { m_port->reset_from_keyboard(state); }

	virtual void receive_data(bool state) = 0;

private:
	// parent port
	required_device<z29_keyboard_port_device> m_port;
};

// device type definition
DECLARE_DEVICE_TYPE(Z29_KEYBOARD, z29_keyboard_port_device)

// standard options
extern void z29_keyboards(device_slot_interface &slot);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline void z29_keyboard_port_device::keyout_w(int state)
{
	 if (m_kbd != nullptr)
		m_kbd->receive_data(state);
}

#endif // MAME_BUS_Z29_KBD_KEYBOARD_H
