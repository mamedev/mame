/**********************************************************************

    Commodore PET/VIC-20/C64/Plus-4 Datassette Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define PET_DATASSETTE_PORT_INTERFACE(_name) \
	const pet_datassette_port_interface (_name) =


#define MCFG_PET_DATASSETTE_PORT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, PET_DATASSETTE_PORT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_datassette_port_interface

struct pet_datassette_port_interface
{
	devcb_write_line    m_out_read_cb;
};


// ======================> pet_datassette_port_device

class device_pet_datassette_port_interface;

class pet_datassette_port_device : public device_t,
									public pet_datassette_port_interface,
									public device_slot_interface
{
public:
	// construction/destruction
	pet_datassette_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pet_datassette_port_device();

	// computer interface
	DECLARE_READ_LINE_MEMBER( read );
	DECLARE_WRITE_LINE_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( sense_r );
	DECLARE_WRITE_LINE_MEMBER( motor_w );

	// device interface
	DECLARE_WRITE_LINE_MEMBER( read_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	devcb_resolved_write_line   m_out_read_func;

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



#endif
