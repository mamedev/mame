// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_SMS_CTRL_SMSCTRL_H
#define MAME_BUS_SMS_CTRL_SMSCTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_control_port_device

class device_sms_control_port_interface;

class sms_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	sms_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sms_control_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	sms_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sms_control_port_device();

	// static configuration helpers
	auto th_input_handler() { return m_th_pin_handler.bind(); }

	// Physical DE-9 connector interface

	// Data returned by the port_r methods:
	// bit 0 - pin 1 - Up
	// bit 1 - pin 2 - Down
	// bit 2 - pin 3 - Left
	// bit 3 - pin 4 - Right
	// bit 4 - pin 5 - Vcc (no data)
	// bit 5 - pin 6 - TL (Button 1/Light Phaser Trigger)
	// bit 6 - pin 7 - TH (Light Phaser sensor)
	//         pin 8 - GND
	// bit 7 - pin 9 - TR (Button 2)
	//
	uint8_t port_r();
	void port_w( uint8_t data );

	void th_pin_w(int state);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// for peripherals that interact with the machine's screen
	required_device<screen_device> m_screen;

protected:
	// device-level overrides
	virtual void device_start() override;

	device_sms_control_port_interface *m_device;

private:
	devcb_write_line m_th_pin_handler;
};


// ======================> device_sms_control_port_interface

// class representing interface-specific live sms_expansion card
class device_sms_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_sms_control_port_interface();

	virtual uint8_t peripheral_r() { return 0xff; }
	virtual void peripheral_w(uint8_t data) { }

protected:
	device_sms_control_port_interface(const machine_config &mconfig, device_t &device);

	sms_control_port_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_CONTROL_PORT, sms_control_port_device)


void sms_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_SMS_CTRL_SMSCTRL_H
