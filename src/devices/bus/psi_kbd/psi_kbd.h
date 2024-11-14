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

// supported devices
void psi_keyboard_devices(device_slot_interface &device);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_psi_keyboard_interface;

// ======================> psi_keyboard_bus_device

class psi_keyboard_bus_device : public device_t, public device_single_card_slot_interface<device_psi_keyboard_interface>
{
public:
	// construction/destruction
	psi_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *dflt)
		: psi_keyboard_bus_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		psi_keyboard_devices(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	psi_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~psi_keyboard_bus_device();

	// callbacks
	auto rx() { return m_rx_handler.bind(); }
	auto key_strobe() { return m_key_strobe_handler.bind(); }

	// called from keyboard
	void rx_w(int state) { m_rx_handler(state); }
	void key_strobe_w(int state) { m_key_strobe_handler(state); }
	void key_data_w(uint8_t data) { m_key_data = data; }

	// called from host
	void tx_w(int state);
	uint8_t key_data_r() { return m_key_data; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	device_psi_keyboard_interface *m_kbd;

	devcb_write_line m_rx_handler;
	devcb_write_line m_key_strobe_handler;

	uint8_t m_key_data;
};

// ======================> device_psi_keyboard_interface

class device_psi_keyboard_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_psi_keyboard_interface();

	virtual void tx_w(int state) { }

protected:
	device_psi_keyboard_interface(const machine_config &mconfig, device_t &device);

	psi_keyboard_bus_device *m_host;
};

// device type definition
DECLARE_DEVICE_TYPE(PSI_KEYBOARD_INTERFACE, psi_keyboard_bus_device)

#endif // MAME_BUS_PSI_KBD_PSI_KBD_H
