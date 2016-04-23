// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   Sega Saturn controller port emulation

**********************************************************************/


#pragma once

#ifndef __SATURN_CONTROL_PORT__
#define __SATURN_CONTROL_PORT__

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class saturn_control_port_device;

// ======================> device_saturn_control_port_interface

class device_saturn_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_saturn_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_saturn_control_port_interface();

	virtual UINT16 read_direct() { return 0; };
	virtual UINT8 read_ctrl(UINT8 offset) { return 0; };
	virtual UINT8 read_status() { return 0xf0; };
	virtual UINT8 read_id(int idx) { return 0xff; };

protected:
	UINT8 m_ctrl_id;
	saturn_control_port_device *m_port;
};

// ======================> saturn_control_port_device

class saturn_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	saturn_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~saturn_control_port_device();

	UINT16 read_direct();
    UINT8 read_ctrl(UINT8 offset);
	UINT8 read_status();
	UINT8 read_id(int idx);

	// device-level overrides
	virtual void device_start() override;
	
protected:
	device_saturn_control_port_interface *m_device;
};


// device type definition
extern const device_type SATURN_CONTROL_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SATURN_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SATURN_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


SLOT_INTERFACE_EXTERN( saturn_controls );
SLOT_INTERFACE_EXTERN( saturn_joys );


#endif
