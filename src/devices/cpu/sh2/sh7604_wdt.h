// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 Watchdog Timer Controller

***************************************************************************/

#pragma once

#ifndef __SH7604_WDTDEV_H__
#define __SH7604_WDTDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SH7604_WDT_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SH7604_WDT, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_wdt_device

class sh7604_wdt_device : public device_t,
						  public device_memory_interface
{
public:
	// construction/destruction
	sh7604_wdt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	const address_space_config      m_space_config;
};


// device type definition
extern const device_type SH7604_WDT;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
