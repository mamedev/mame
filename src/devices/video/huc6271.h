// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

***************************************************************************/

#pragma once

#ifndef __HUC6271DEV_H__
#define __HUC6271DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HUC6271_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, huc6271, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> huc6271_device

class huc6271_device : public device_t
{
public:
	// construction/destruction
	huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	DECLARE_ADDRESS_MAP(regs, 16);

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
extern const device_type huc6271;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
