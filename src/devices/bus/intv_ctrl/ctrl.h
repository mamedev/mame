// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   Mattel Intellivision controller port emulation

**********************************************************************/


#pragma once

#ifndef __INTV_CONTROL_PORT__
#define __INTV_CONTROL_PORT__

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class intv_control_port_device;

// ======================> device_intv_control_port_interface

class device_intv_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_intv_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_intv_control_port_interface();

	virtual UINT8 read_ctrl() { return 0; };

protected:
	intv_control_port_device *m_port;
};

// ======================> intv_control_port_device

class intv_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	intv_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~intv_control_port_device();

	DECLARE_READ8_MEMBER( ctrl_r ) { return read_ctrl(); }
	UINT8 read_ctrl();

protected:
	// device-level overrides
	virtual void device_start() override;

	device_intv_control_port_interface *m_device;
};


// device type definition
extern const device_type INTV_CONTROL_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTV_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, INTV_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



SLOT_INTERFACE_EXTERN( intv_control_port_devices );


#endif
