/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __huc6272DEV_H__
#define __huc6272DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HUC6272_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, huc6272, _freq) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> huc6272_device

class huc6272_device :	public device_t
{
public:
	// construction/destruction
	huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
};


// device type definition
extern const device_type huc6272;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
