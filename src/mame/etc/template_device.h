// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton device

***************************************************************************/

#ifndef MAME_MACHINE_XXX_H
#define MAME_MACHINE_XXX_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xxx_device

class xxx_device : public device_t
{
public:
	// construction/destruction
	xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(XXX, xxx_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_XXX_H
