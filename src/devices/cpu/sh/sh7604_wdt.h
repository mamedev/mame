// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 Watchdog Timer Controller

***************************************************************************/

#ifndef MAME_CPU_SH7604_WDT_H
#define MAME_CPU_SH7604_WDT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_wdt_device

class sh7604_wdt_device : public device_t
{
public:
	// construction/destruction
	sh7604_wdt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void wdt_regs(address_map &map) ATTR_COLD;

	void write(address_space &space, offs_t offset, uint16_t data);
	uint8_t read(address_space &space, offs_t offset);

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	const address_space_config      m_space_config;
};


// device type definition
DECLARE_DEVICE_TYPE(SH7604_WDT, sh7604_wdt_device)

#endif // MAME_CPU_SH7604_WDT_H
