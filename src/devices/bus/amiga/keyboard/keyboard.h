// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Keyboard Interface

    Serial data and reset control

    - KDAT (serial data)
    - KCLK (serial clock)
    - KRST (reset output)

***************************************************************************/

#ifndef MAME_BUS_AMIGA_KEYBOARD_H
#define MAME_BUS_AMIGA_KEYBOARD_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_amiga_keyboard_interface;

// ======================> amiga_keyboard_bus_device

class amiga_keyboard_bus_device : public device_t, public device_single_card_slot_interface<device_amiga_keyboard_interface>
{
public:
	// construction/destruction
	template <typename T>
	amiga_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: amiga_keyboard_bus_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	amiga_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~amiga_keyboard_bus_device();

	// callbacks
	auto kclk_handler() { return m_kclk_handler.bind(); }
	auto kdat_handler() { return m_kdat_handler.bind(); }
	auto krst_handler() { return m_krst_handler.bind(); }

	// called from keyboard
	void kclk_w(int state) { m_kclk_handler(state); }
	void kdat_w(int state) { m_kdat_handler(state); }
	void krst_w(int state) { m_krst_handler(state); }

	// called from host
	void kdat_in_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	device_amiga_keyboard_interface *m_kbd;

	devcb_write_line m_kclk_handler;
	devcb_write_line m_kdat_handler;
	devcb_write_line m_krst_handler;
};

// ======================> device_amiga_keyboard_interface

class device_amiga_keyboard_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_amiga_keyboard_interface();

	virtual void kdat_w(int state) = 0;

protected:
	device_amiga_keyboard_interface(const machine_config &mconfig, device_t &device);

	amiga_keyboard_bus_device *m_host;
};

// device type definition
DECLARE_DEVICE_TYPE(AMIGA_KEYBOARD_INTERFACE, amiga_keyboard_bus_device)

// supported devices
void amiga_keyboard_devices(device_slot_interface &device);
void a500_keyboard_devices(device_slot_interface &device);
void a600_keyboard_devices(device_slot_interface &device);

#endif // MAME_BUS_AMIGA_KEYBOARD_H
