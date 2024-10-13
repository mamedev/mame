// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics RAM Disc

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_RAMDISC_H
#define MAME_BUS_BBC_1MHZBUS_RAMDISC_H

#include "1mhzbus.h"
#include "machine/nvram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_ramdisc_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_ramdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(power_changed);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<nvram_device> m_nvram;
	required_ioport m_ram_size;
	required_ioport m_power;

	std::unique_ptr<uint8_t[]> m_ram;
	uint16_t m_sector;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_RAMDISC, bbc_ramdisc_device);


#endif /* MAME_BUS_BBC_1MHZBUS_RAMDISC_H */
