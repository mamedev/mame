// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Sperry Univac UTS series keyboard port

***************************************************************************/

#ifndef MAME_BUS_UTS_KBD_UTS_KBD_H
#define MAME_BUS_UTS_KBD_UTS_KBD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_uts_keyboard_interface;

// ======================> uts_keyboard_port_device

class uts_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_uts_keyboard_interface>
{
	friend class device_uts_keyboard_interface;

public:
	// construction/destruction
	uts_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	uts_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: uts_keyboard_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// configuration
	auto rxd_callback() { return m_rxd_callback.bind(); }

	// line handler
	inline void ready_w(int state);

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	// called from keyboard
	void write_rxd(int state) { m_rxd_callback(state); }

private:
	// user callback
	devcb_write_line m_rxd_callback;

	// selected keyboard
	device_uts_keyboard_interface *m_kbd;
};

// ======================> device_uts_keyboard_interface

class device_uts_keyboard_interface : public device_interface
{
	friend class uts_keyboard_port_device;

protected:
	// construction/destruction
	device_uts_keyboard_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_uts_keyboard_interface();

	void write_rxd(int state) { m_port->write_rxd(state); }

	virtual void ready_w(int state) = 0;

private:
	// parent port
	required_device<uts_keyboard_port_device> m_port;
};

// type definition
DECLARE_DEVICE_TYPE(UTS_KEYBOARD, uts_keyboard_port_device)

// standard options
extern void uts20_keyboards(device_slot_interface &slot);
extern void uts10_keyboards(device_slot_interface &slot);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

void uts_keyboard_port_device::ready_w(int state)
{
	 if (m_kbd != nullptr)
		m_kbd->ready_w(state);
}

#endif // MAME_BUS_UTS_KBD_UTS_KBD_H
