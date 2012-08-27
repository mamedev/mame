/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __M6M80011APDEV_H__
#define __M6M80011APDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

/* TODO: frequency */
#define MCFG_M6M80011AP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M6M80011AP, XTAL_32_768kHz) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m6m80011ap_device

class m6m80011ap_device :	public device_t
{
public:
	// construction/destruction
	m6m80011ap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	READ_LINE_MEMBER( read_bit );
	READ_LINE_MEMBER( ready_line );
	WRITE_LINE_MEMBER( set_cs_line );
	WRITE_LINE_MEMBER( set_clock_line );
	WRITE_LINE_MEMBER( write_bit );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

};


// device type definition
extern const device_type M6M80011AP;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
