// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear EXT port emulation
    Also known as Gear-to-Gear (or VS, in Japan) cable connector

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __GG_EXT_PORT__
#define __GG_EXT_PORT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GG_EXT_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, GG_EXT_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)
#define MCFG_GG_EXT_PORT_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)


#define MCFG_GG_EXT_PORT_TH_INPUT_HANDLER(_devcb) \
	devcb = &gg_ext_port_device::set_th_input_handler(*device, DEVCB_##_devcb);


#define MCFG_GG_EXT_PORT_PIXEL_HANDLER(_devcb) \
	devcb = &gg_ext_port_device::set_pixel_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gg_ext_port_device

class device_gg_ext_port_interface;

class gg_ext_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	gg_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~gg_ext_port_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_th_input_handler(device_t &device, _Object object) { return downcast<gg_ext_port_device &>(device).m_th_pin_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_pixel_handler(device_t &device, _Object object) { return downcast<gg_ext_port_device &>(device).m_pixel_handler.set_callback(object); }

	// Currently, only the support for SMS Controller Adaptor is emulated,
	// for when SMS Compatibility mode is enabled. In that mode, the 10 pins
	// of the EXT port follows the same numbering of a SMS Control port.

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
	//         pin 10 - Not connected
	//
	UINT8 port_r();
	void port_w( UINT8 data );

	void th_pin_w(int state);
	UINT32 pixel_r();

//protected:
	// device-level overrides
	virtual void device_start();

	device_gg_ext_port_interface *m_device;

private:
	devcb_write_line m_th_pin_handler;
	devcb_read32 m_pixel_handler;
};


// ======================> device_gg_ext_port_interface

// class representing interface-specific live sms_expansion card
class device_gg_ext_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_gg_ext_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_gg_ext_port_interface();

	virtual UINT8 peripheral_r() { return 0xff; };
	virtual void peripheral_w(UINT8 data) { };

protected:
	gg_ext_port_device *m_port;
};


// device type definition
extern const device_type GG_EXT_PORT;


SLOT_INTERFACE_EXTERN( gg_ext_port_devices );


#endif
