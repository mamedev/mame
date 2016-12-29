// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __NAMCO_C148DEV_H__
#define __NAMCO_C148DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C148_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C148, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c148_device

class namco_c148_device : public device_t
{
public:
	// construction/destruction
	namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_ADDRESS_MAP(map, 16);

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// ...
};


// device type definition
extern const device_type NAMCO_C148;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
