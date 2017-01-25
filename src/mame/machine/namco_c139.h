// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C139 - Serial I/F Controller

***************************************************************************/

#pragma once

#ifndef __NAMCO_C139DEV_H__
#define __NAMCO_C139DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C139_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C139, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c139_device

class namco_c139_device : public device_t,
						  public device_memory_interface
{
public:
	// construction/destruction
	namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	DECLARE_ADDRESS_MAP(regs_map, 16);

	DECLARE_READ16_MEMBER(status_r);

	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_DATA) const override;
private:
	const address_space_config m_space_config;
	uint16_t* m_ram;
};


// device type definition
extern const device_type NAMCO_C139;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
