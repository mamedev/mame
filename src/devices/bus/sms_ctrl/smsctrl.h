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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMS_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SMS_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)
#define MCFG_SMS_CONTROL_PORT_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)


#define MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(_devcb) \
	downcast<sms_control_port_device &>(*device).set_th_input_handler(DEVCB_##_devcb);


#define MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(_devcb) \
	downcast<sms_control_port_device &>(*device).set_pixel_handler(DEVCB_##_devcb);



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
	sms_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sms_control_port_device();

	// static configuration helpers
	template <class Object> devcb_base &set_th_input_handler(Object &&cb) { return m_th_pin_handler.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_pixel_handler(Object &&cb) { return m_pixel_handler.set_callback(std::forward<Object>(cb)); }

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
	uint32_t pixel_r();

protected:
	// device-level overrides
	virtual void device_start() override;

	device_sms_control_port_interface *m_device;

private:
	devcb_write_line m_th_pin_handler;
	devcb_read32 m_pixel_handler;
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
