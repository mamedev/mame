// license: GPL-2.0+
// copyright-holders: Dirk Best
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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AMIGA_KEYBOARD_INTERFACE_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, AMIGA_KEYBOARD_INTERFACE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(amiga_keyboard_devices, _def_slot, false)

#define MCFG_AMIGA_KEYBOARD_KCLK_HANDLER(_devcb) \
	devcb = &downcast<amiga_keyboard_bus_device &>(*device).set_kclk_handler(DEVCB_##_devcb);

#define MCFG_AMIGA_KEYBOARD_KDAT_HANDLER(_devcb) \
	devcb = &downcast<amiga_keyboard_bus_device &>(*device).set_kdat_handler(DEVCB_##_devcb);

#define MCFG_AMIGA_KEYBOARD_KRST_HANDLER(_devcb) \
	devcb = &downcast<amiga_keyboard_bus_device &>(*device).set_krst_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_amiga_keyboard_interface;

// ======================> amiga_keyboard_bus_device

class amiga_keyboard_bus_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	amiga_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~amiga_keyboard_bus_device();

	// callbacks
	template <class Object> devcb_base &set_kclk_handler(Object &&cb) { return m_kclk_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_kdat_handler(Object &&cb) { return m_kdat_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_krst_handler(Object &&cb) { return m_krst_handler.set_callback(std::forward<Object>(cb)); }

	// called from keyboard
	DECLARE_WRITE_LINE_MEMBER(kclk_w) { m_kclk_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(kdat_w) { m_kdat_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(krst_w) { m_krst_handler(state); }

	// called from host
	DECLARE_WRITE_LINE_MEMBER(kdat_in_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	device_amiga_keyboard_interface *m_kbd;

	devcb_write_line m_kclk_handler;
	devcb_write_line m_kdat_handler;
	devcb_write_line m_krst_handler;
};

// ======================> device_amiga_keyboard_interface

class device_amiga_keyboard_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_amiga_keyboard_interface();

	virtual DECLARE_WRITE_LINE_MEMBER(kdat_w) = 0;

protected:
	device_amiga_keyboard_interface(const machine_config &mconfig, device_t &device);

	amiga_keyboard_bus_device *m_host;
};

// device type definition
DECLARE_DEVICE_TYPE(AMIGA_KEYBOARD_INTERFACE, amiga_keyboard_bus_device)

// supported devices
void amiga_keyboard_devices(device_slot_interface &device);

#endif // MAME_BUS_AMIGA_KEYBOARD_H
