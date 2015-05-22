// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __XXXDEV_H__
#define __XXXDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_XXX_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, XXX, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xxx_device

class xxx_device : public device_t
{
public:
	// construction/destruction
	xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
extern const device_type XXX;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
