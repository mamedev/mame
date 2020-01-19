// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    PMS 64K Non-Volatile Ram Module

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_PMS64K_H
#define MAME_BUS_BBC_1MHZBUS_PMS64K_H

#include "1mhzbus.h"
#include "machine/nvram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_pms64k_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_pms64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_ram_page;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_PMS64K, bbc_pms64k_device);


#endif /* MAME_BUS_BBC_1MHZBUS_PMS64K_H */
