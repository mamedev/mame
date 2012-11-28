/***************************************************************************

Acorn Archimedes KART interface

***************************************************************************/

#pragma once

#ifndef __AAKARTDEV_H__
#define __AAKARTDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AAKART_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, AAKART, _freq) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aakart_device

class aakart_device : public device_t
{
public:
	// construction/destruction
	aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
};


// device type definition
extern const device_type AAKART;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
