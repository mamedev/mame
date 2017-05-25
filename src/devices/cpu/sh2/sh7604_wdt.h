// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 Watchdog Timer Controller

***************************************************************************/

#ifndef MAME_CPU_SH7604_WDT_H
#define MAME_CPU_SH7604_WDT_H

#pragma once



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
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

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
DECLARE_DEVICE_TYPE(SH7604_WDT, sh7604_wdt_device)

#endif // MAME_CPU_SH7604_WDT_H
