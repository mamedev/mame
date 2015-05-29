// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET/VIC-20/C64/Plus-4 Datassette Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       +5V
                  MOTOR       3      C       MOTOR
                   READ       4      D       READ
                  WRITE       5      E       WRITE
                  SENSE       6      F       SENSE

**********************************************************************/

#pragma once

#ifndef __PET_DATASSETTE_PORT__
#define __PET_DATASSETTE_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PET_DATASSETTE_PORT_TAG     "tape"
#define PET_DATASSETTE_PORT2_TAG     "tape2"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PET_DATASSETTE_PORT_ADD(_tag, _slot_intf, _def_slot, _devcb) \
	MCFG_DEVICE_ADD(_tag, PET_DATASSETTE_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	devcb = &pet_datassette_port_device::set_read_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_datassette_port_device

class device_pet_datassette_port_interface;

class pet_datassette_port_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	pet_datassette_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pet_datassette_port_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_handler(device_t &device, _Object object) { return downcast<pet_datassette_port_device &>(device).m_read_handler.set_callback(object); }

	// computer interface
	DECLARE_READ_LINE_MEMBER( read );
	DECLARE_WRITE_LINE_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( sense_r );
	DECLARE_WRITE_LINE_MEMBER( motor_w );

	// device interface
	DECLARE_WRITE_LINE_MEMBER( read_w );

protected:
	// device-level overrides
	virtual void device_start();

	devcb_write_line m_read_handler;

	device_pet_datassette_port_interface *m_cart;
};


// ======================> device_pet_datassette_port_interface

// class representing interface-specific live c64_expansion card
class device_pet_datassette_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_pet_datassette_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pet_datassette_port_interface();

	virtual int datassette_read() { return 1; };
	virtual void datassette_write(int state) { };
	virtual int datassette_sense() { return 1; };
	virtual void datassette_motor(int state) { };

protected:
	pet_datassette_port_device *m_slot;
};


// device type definition
extern const device_type PET_DATASSETTE_PORT;


// slot devices
#include "c2n.h"
#include "diag264_lb_tape.h"

SLOT_INTERFACE_EXTERN( cbm_datassette_devices );



#endif
