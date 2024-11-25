// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Wyse Keyboard Port interface

***************************************************************************/

#ifndef MAME_BUS_WYSEKBD_KEYBOARD_H
#define MAME_BUS_WYSEKBD_KEYBOARD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class wyse_keyboard_interface;

// ======================> wyse_keyboard_port_device

class wyse_keyboard_port_device : public device_t, public device_single_card_slot_interface<wyse_keyboard_interface>
{
public:
	// construction/destruction
	wyse_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	wyse_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: wyse_keyboard_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// line handlers
	void cmd_w(int state);
	int data_r();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

private:
	// selected keyboard
	wyse_keyboard_interface *m_kbd;
};

// ======================> wyse_keyboard_interface

class wyse_keyboard_interface : public device_interface
{
	friend class wyse_keyboard_port_device;

protected:
	// construction/destruction
	wyse_keyboard_interface(const machine_config &mconfig, device_t &device);
	virtual ~wyse_keyboard_interface();

	virtual bool wysekbd_read_data() = 0;
	virtual void wysekbd_write_cmd(bool state) = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(WYSE_KEYBOARD, wyse_keyboard_port_device)

// standard options
extern void wy85_keyboards(device_slot_interface &slot);
extern void wy30_keyboards(device_slot_interface &slot);
extern void wy60_keyboards(device_slot_interface &slot);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline void wyse_keyboard_port_device::cmd_w(int state)
{
	if (m_kbd != nullptr)
		m_kbd->wysekbd_write_cmd(state);
}

inline int wyse_keyboard_port_device::data_r()
{
	if (m_kbd != nullptr)
		return m_kbd->wysekbd_read_data();
	else
		return 1;
}

#endif // MAME_BUS_WYSEKBD_KEYBOARD_H
