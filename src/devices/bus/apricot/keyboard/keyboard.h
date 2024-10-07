// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard Interface

    Host interface: 9-pin D-SUB

    1  +12V
    2  OUT
    3  IN
    4  N/C
    5  N/C
    6  GND
    7  -12V
    8  0V
    9  N/C

    Keyboard interface:

    A  0V
    B  +12V
    C  -12V
    D  N/C
    E  OUT
    F  IN

***************************************************************************/

#ifndef MAME_BUS_APRICOT_KEYBOARD_KEYBOARD_H
#define MAME_BUS_APRICOT_KEYBOARD_KEYBOARD_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_apricot_keyboard_interface;

// ======================> apricot_keyboard_bus_device

class apricot_keyboard_bus_device : public device_t, public device_single_card_slot_interface<device_apricot_keyboard_interface>
{
public:
	// construction/destruction
	template <typename T>
	apricot_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: apricot_keyboard_bus_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	apricot_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~apricot_keyboard_bus_device();

	// callbacks
	auto in_handler() { return m_in_handler.bind(); }

	// called from keyboard
	void in_w(int state) { m_in_handler(state); }

	// called from host
	void out_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	device_apricot_keyboard_interface *m_kbd;

	devcb_write_line m_in_handler;
};

// ======================> device_apricot_keyboard_interface

class device_apricot_keyboard_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_apricot_keyboard_interface();

	virtual void out_w(int state) = 0;

protected:
	device_apricot_keyboard_interface(const machine_config &mconfig, device_t &device);

	apricot_keyboard_bus_device *m_host;
};


// device type definition
DECLARE_DEVICE_TYPE(APRICOT_KEYBOARD_INTERFACE, apricot_keyboard_bus_device)

// supported devices
void apricot_keyboard_devices(device_slot_interface &device);


#endif // MAME_BUS_APRICOT_KEYBOARD_KEYBOARD_H
