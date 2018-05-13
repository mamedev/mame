// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Kontron PSI keyboard interface

    This interface supports both a parallel and a serial keyboard

     9  KEY.D0
     8  KEY.D1
     7  KEY.D2
     6  KEY.D3
     5  KEY.D4
     4  KEY.D5
     3  KEY.D6
     2  KEY.D7
    11  KEY.STRB
    15  + KEY.TRANSM
    16  - KEY.TRANSM
    17  + KEY.REC
    18  + KEY.REC
    13  + 5V
    14  GND
     1  Shield

***************************************************************************/

#ifndef MAME_BUS_PSI_KBD_PSI_KBD_H
#define MAME_BUS_PSI_KBD_PSI_KBD_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PSI_KEYBOARD_INTERFACE_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PSI_KEYBOARD_INTERFACE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(psi_keyboard_devices, _def_slot, false)

#define MCFG_PSI_KEYBOARD_RX_HANDLER(_devcb) \
	devcb = &downcast<psi_keyboard_bus_device &>(*device).set_rx_handler(DEVCB_##_devcb);

#define MCFG_PSI_KEYBOARD_KEY_STROBE_HANDLER(_devcb) \
	devcb = &downcast<psi_keyboard_bus_device &>(*device).set_key_strobe_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_psi_keyboard_interface;

// ======================> psi_keyboard_bus_device

class psi_keyboard_bus_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	psi_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~psi_keyboard_bus_device();

	// callbacks
	template <class Object> devcb_base &set_rx_handler(Object &&cb) { return m_rx_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_key_strobe_handler(Object &&cb) { return m_key_strobe_handler.set_callback(std::forward<Object>(cb)); }

	// called from keyboard
	DECLARE_WRITE_LINE_MEMBER( rx_w ) { m_rx_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( key_strobe_w ) { m_key_strobe_handler(state); }
	DECLARE_WRITE8_MEMBER( key_data_w ) { m_key_data = data; }

	// called from host
	DECLARE_WRITE_LINE_MEMBER( tx_w );
	DECLARE_READ8_MEMBER( key_data_r ) { return m_key_data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	device_psi_keyboard_interface *m_kbd;

	devcb_write_line m_rx_handler;
	devcb_write_line m_key_strobe_handler;

	uint8_t m_key_data;
};

// ======================> device_psi_keyboard_interface

class device_psi_keyboard_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_psi_keyboard_interface();

	virtual void tx_w(int state) {};

protected:
	device_psi_keyboard_interface(const machine_config &mconfig, device_t &device);

	psi_keyboard_bus_device *m_host;
};


// device type definition
DECLARE_DEVICE_TYPE(PSI_KEYBOARD_INTERFACE, psi_keyboard_bus_device)

// supported devices
void psi_keyboard_devices(device_slot_interface &device);


#endif // MAME_BUS_PSI_KBD_PSI_KBD_H
