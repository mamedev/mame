/**********************************************************************

    RCA Cosmac VIP Byte Input/Output port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                            A   IN 0
                            B   IN 1
                            C   IN 2
                            D   IN 3
                            E   IN 4
                            F   IN 5
                            H   IN 6
                            J   IN 7
                            K   INST
                            L   _EF4
                            M   OUT 0
                            N   OUT 1
                            P   OUT 2
                            R   OUT 3
                            S   OUT 4
                            T   OUT 5
                            U   OUT 6
                            V   OUT 7
                            W   Q
                            X   _EF3
                            Y   +5 V
                            Z   GND

**********************************************************************/

#pragma once

#ifndef __VIP_BYTEIO_PORT__
#define __VIP_BYTEIO_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIP_BYTEIO_PORT_TAG     "byteio"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define VIP_BYTEIO_PORT_INTERFACE(_name) \
	const vip_byteio_port_interface (_name) =


#define MCFG_VIP_BYTEIO_PORT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, VIP_BYTEIO_PORT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vip_byteio_port_interface

struct vip_byteio_port_interface
{
	devcb_write_line    m_out_inst_cb;
};


// ======================> vip_byteio_port_device

class device_vip_byteio_port_interface;

class vip_byteio_port_device : public device_t,
								public vip_byteio_port_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	vip_byteio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~vip_byteio_port_device();

	// computer interface
	UINT8 in_r();
	void out_w(UINT8 data);
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( inst_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	devcb_resolved_write_line   m_out_inst_func;

	device_vip_byteio_port_interface *m_cart;
};


// ======================> device_vip_byteio_port_interface

// class representing interface-specific live c64_expansion card
class device_vip_byteio_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vip_byteio_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vip_byteio_port_interface();

	virtual UINT8 vip_in_r() { return 0xff; };
	virtual void vip_out_w(UINT8 data) { };

	virtual int vip_ef3_r() { return CLEAR_LINE; }
	virtual int vip_ef4_r() { return CLEAR_LINE; }

	virtual void vip_q_w(int state) { };

protected:
	vip_byteio_port_device *m_slot;
};


// device type definition
extern const device_type VIP_BYTEIO_PORT;


// slot devices
#include "machine/vp620.h"

SLOT_INTERFACE_EXTERN( vip_byteio_cards );



#endif
