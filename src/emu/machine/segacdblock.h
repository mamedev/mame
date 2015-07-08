// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __SEGACDBLOCKDEV_H__
#define __SEGACDBLOCKDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGACDBLOCK_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SEGACDBLOCK, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> segacdblock_device

class segacdblock_device : public device_t
{
public:
	// construction/destruction
	segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
extern const device_type SEGACDBLOCK;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
