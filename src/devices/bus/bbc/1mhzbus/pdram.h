// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Pull Down RAM

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_PDRAM_H
#define MAME_BUS_BBC_1MHZBUS_PDRAM_H

#include "1mhzbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_pdram_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_pdram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_ram_page;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_PDRAM, bbc_pdram_device);


#endif /* MAME_BUS_BBC_1MHZBUS_PDRAM_H */
