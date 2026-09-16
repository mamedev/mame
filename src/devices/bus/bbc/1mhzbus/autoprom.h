// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ATPL AutoPrommer - Eprom Programmer with Auto-Run

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_AUTOPROM_H
#define MAME_BUS_BBC_1MHZBUS_AUTOPROM_H

#include "1mhzbus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_autoprom_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_autoprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;

private:
	required_memory_region m_autorun;
	required_ioport m_boot;

	uint16_t m_rom_offset;
	uint8_t m_unk_c0;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_AUTOPROM, bbc_autoprom_device)


#endif // MAME_BUS_BBC_1MHZBUS_AUTOPROM_H
