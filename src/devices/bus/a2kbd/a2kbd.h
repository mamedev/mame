// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II Keyboard Connector

*********************************************************************/

#ifndef MAME_BUS_A2KBD_A2KBD_H
#define MAME_BUS_A2KBD_A2KBD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_a2kbd_interface;

// ======================> a2kbd_connector_device

class a2kbd_connector_device : public device_t, public device_single_card_slot_interface<device_a2kbd_interface>
{
	friend class device_a2kbd_interface;

public:
	// construction/destruction
	a2kbd_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T>
	a2kbd_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: a2kbd_connector_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// standard options
	static void default_options(device_slot_interface &slot);

	// output callback configuration
	auto b_callback() { return m_b_callback.bind(); }
	auto strobe_callback() { return m_strobe_callback.bind(); }
	auto reset_callback() { return m_reset_callback.bind(); }
	auto mode_callback() { return m_mode_callback.bind(); }

	// shift and control key inputs (modification)
	int shift_r();
	int control_r();

	// outputs to keyboard (modification)
	void ack_w(int state);
	void an3_w(int state);

protected:
	// device_t implementation
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// callback objects
	devcb_write8 m_b_callback;
	devcb_write_line m_strobe_callback;
	devcb_write_line m_reset_callback;
	devcb_write_line m_mode_callback;

	// selected device
	device_a2kbd_interface *m_intf;
};

// ======================> device_a2kbd_interface

class device_a2kbd_interface : public device_interface
{
	friend class a2kbd_connector_device;

public:
	virtual ~device_a2kbd_interface();

protected:
	// construction/destruction
	device_a2kbd_interface(const machine_config &mconfig, device_t &device);

	// dedicated inputs from keyboard
	virtual int shift_r() { return 0; }
	virtual int control_r() { return 0; }

	// optional outputs to keyboard
	virtual void ack_w(int state) { }
	virtual void an3_w(int state) { }

	void b_w(u8 data) { m_connector->m_b_callback(data & 0x7f); }
	void strobe_w(int state) { m_connector->m_strobe_callback(state); }
	void reset_w(int state) { m_connector->m_reset_callback(state); }
	void mode_w(int state) { m_connector->m_mode_callback(state); }

private:
	a2kbd_connector_device *m_connector;
};

// device type declaration
DECLARE_DEVICE_TYPE(A2KBD_CONNECTOR, a2kbd_connector_device)

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline int a2kbd_connector_device::shift_r()
{
	// Poll shift key(s) (0 = pressed, 1 = not pressed)
	return (m_intf != nullptr) ? m_intf->shift_r() : 0;
}

inline int a2kbd_connector_device::control_r()
{
	// Poll control key(s) (0 = pressed, 1 = not pressed)
	return (m_intf != nullptr) ? m_intf->control_r() : 1;
}

inline void a2kbd_connector_device::ack_w(int state)
{
	if (m_intf != nullptr)
		m_intf->ack_w(state);
}

inline void a2kbd_connector_device::an3_w(int state)
{
	if (m_intf != nullptr)
		m_intf->an3_w(state);
}

#endif // MAME_BUS_A2KBD_A2KBD_H
