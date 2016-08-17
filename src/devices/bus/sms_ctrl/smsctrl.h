// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System controller port emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __SMS_CONTROL_PORT__
#define __SMS_CONTROL_PORT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMS_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SMS_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)
#define MCFG_SMS_CONTROL_PORT_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)


#define MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(_devcb) \
	devcb = &sms_control_port_device::set_th_input_handler(*device, DEVCB_##_devcb);


#define MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(_devcb) \
	devcb = &sms_control_port_device::set_pixel_handler(*device, DEVCB_##_devcb);



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
	sms_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~sms_control_port_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_th_input_handler(device_t &device, _Object object) { return downcast<sms_control_port_device &>(device).m_th_pin_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_pixel_handler(device_t &device, _Object object) { return downcast<sms_control_port_device &>(device).m_pixel_handler.set_callback(object); }

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
	UINT8 port_r();
	void port_w( UINT8 data );

	void th_pin_w(int state);
	UINT32 pixel_r();

//protected:
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
	device_sms_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sms_control_port_interface();

	virtual UINT8 peripheral_r() { return 0xff; };
	virtual void peripheral_w(UINT8 data) { };

protected:
	sms_control_port_device *m_port;
};


// device type definition
extern const device_type SMS_CONTROL_PORT;


SLOT_INTERFACE_EXTERN( sms_control_port_devices );


#endif
