// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 800/802/806/1600 keyboard port emulation

**********************************************************************/

#pragma once

#ifndef __ABC_KEYBOARD_PORT__
#define __ABC_KEYBOARD_PORT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC_KEYBOARD_PORT_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ABC_KEYBOARD_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(abc_keyboard_devices, _def_slot, false)

#define MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(_devcb) \
	devcb = &abc_keyboard_port_device::set_out_rx_handler(*device, DEVCB_##_devcb);

#define MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(_devcb) \
	devcb = &abc_keyboard_port_device::set_out_trxc_handler(*device, DEVCB_##_devcb);

#define MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(_devcb) \
	devcb = &abc_keyboard_port_device::set_out_keydown_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class abc_keyboard_interface;

class abc_keyboard_port_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	abc_keyboard_port_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_rx_handler(device_t &device, _Object object) { return downcast<abc_keyboard_port_device &>(device).m_out_rx_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_out_trxc_handler(device_t &device, _Object object) { return downcast<abc_keyboard_port_device &>(device).m_out_trxc_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_out_keydown_handler(device_t &device, _Object object) { return downcast<abc_keyboard_port_device &>(device).m_out_keydown_handler.set_callback(object); }

	// computer interface
	DECLARE_WRITE_LINE_MEMBER( txd_w );

	// peripheral interface
	DECLARE_WRITE_LINE_MEMBER( write_rx );
	DECLARE_WRITE_LINE_MEMBER( trxc_w );
	DECLARE_WRITE_LINE_MEMBER( keydown_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_out_rx_handler;
	devcb_write_line m_out_trxc_handler;
	devcb_write_line m_out_keydown_handler;

	abc_keyboard_interface *m_card;
};


class abc_keyboard_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	abc_keyboard_interface(const machine_config &mconfig, device_t &device);

	virtual void txd_w(int state) { };

protected:
	abc_keyboard_port_device *m_slot;
};


// device type definition
extern const device_type ABC_KEYBOARD_PORT;


// supported devices
SLOT_INTERFACE_EXTERN( abc_keyboard_devices );



#endif
