// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   SNK Neo Geo controller port emulation

**********************************************************************/


#pragma once

#ifndef __NEOGEO_CONTROL_PORT__
#define __NEOGEO_CONTROL_PORT__

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class neogeo_control_port_device;

// ======================> device_neogeo_control_port_interface

class device_neogeo_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_neogeo_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_neogeo_control_port_interface();

	virtual UINT16 read_ctrl() { return 0xffff; };
	virtual UINT8 read_start_sel() { return 0xff; };
	virtual void write_ctrlsel(UINT8 data) { };

protected:
	neogeo_control_port_device *m_port;
};

// ======================> neogeo_control_port_device

class neogeo_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	neogeo_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~neogeo_control_port_device();

    UINT16 read_ctrl();
	UINT8 read_start_sel();
	void write_ctrlsel(UINT8 data);
	DECLARE_READ16_MEMBER( ctrl_r ) { return read_ctrl(); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_neogeo_control_port_interface *m_device;
};


// device type definition
extern const device_type NEOGEO_CONTROL_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NEOGEO_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NEOGEO_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



SLOT_INTERFACE_EXTERN( neogeo_controls );


#endif
