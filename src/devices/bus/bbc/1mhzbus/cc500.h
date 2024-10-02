// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Colour Card 500 - CTS Recognition

**********************************************************************/

#ifndef MAME_BUS_BBC_1MHZBUS_CC500_H
#define MAME_BUS_BBC_1MHZBUS_CC500_H

#include "1mhzbus.h"
#include "machine/6522via.h"
#include "bus/bbc/userport/userport.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_cc500_device

class bbc_cc500_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_cc500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	required_device<palette_device> m_palette;
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<via6522_device> m_via;
	required_device<bbc_userport_slot_device> m_userport;

	rgb_t m_palette_ram[8];
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CC500, bbc_cc500_device)


#endif // MAME_BUS_BBC_1MHZBUS_CC500_H
