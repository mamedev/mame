// license:BSD-3-Clause
// copyright-holders:<author_name>
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
	xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
extern const device_type XXX;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
