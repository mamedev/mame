// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow 24bBC/RAM Disc Board

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_24BBC_H
#define MAME_BUS_BBC_1MHZBUS_24BBC_H

#include "1mhzbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_24bbc_device : public device_t, public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_24bbc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;

	std::unique_ptr<uint8_t[]> m_ram;

	uint32_t m_ram_addr;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_24BBC, bbc_24bbc_device);


#endif /* MAME_BUS_BBC_1MHZBUS_24BBC_H */
