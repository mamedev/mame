// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Generic Keytronic serial keyboard connector

***************************************************************************/

#ifndef MAME_BUS_KEYTRONIC_KEYTRONIC_H
#define MAME_BUS_KEYTRONIC_KEYTRONIC_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_keytronic_interface;

// ======================> keytronic_connector_device

class keytronic_connector_device : public device_t, public device_single_card_slot_interface<device_keytronic_interface>
{
	friend class device_keytronic_interface;

public:
	// construction/destruction
	keytronic_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	keytronic_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: keytronic_connector_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// callback configuration
	auto ser_out_callback() { return m_ser_out_callback.bind(); }

	// serial line input
	inline void ser_in_w(int state);

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	// called from keyboard
	void ser_out_w(int state) { m_ser_out_callback(state); }

private:
	// user callback
	devcb_write_line m_ser_out_callback;

	// selected keyboard
	device_keytronic_interface *m_kbd;
};

// ======================> device_keytronic_interface

class device_keytronic_interface : public device_interface
{
	friend class keytronic_connector_device;

protected:
	// construction/destruction
	device_keytronic_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_keytronic_interface();

	void ser_out_w(int state) { m_port->ser_out_w(state); }

	virtual void ser_in_w(int state) = 0;

private:
	// parent port
	required_device<keytronic_connector_device> m_port;
};

// type definition
DECLARE_DEVICE_TYPE(KEYTRONIC_CONNECTOR, keytronic_connector_device)

// supported keyboards
void ascii_terminal_keyboards(device_slot_interface &device);
void informer_207_100_keyboards(device_slot_interface &device);
void informer_213_keyboards(device_slot_interface &device);
void kaypro_keyboards(device_slot_interface &device);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

void keytronic_connector_device::ser_in_w(int state)
{
	 if (m_kbd != nullptr)
		m_kbd->ser_in_w(state);
}

#endif // MAME_BUS_KEYTRONIC_KEYTRONIC_H
