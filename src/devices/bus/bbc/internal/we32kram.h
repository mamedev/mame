// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics 32K RAM card

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_32KRAMcard.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_WE32KRAM_H
#define MAME_BUS_BBC_INTERNAL_WE32KRAM_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_we32kram_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_we32kram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_ram() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;

private:
	void control_w(offs_t offset, uint8_t data);

	uint8_t m_shadow;
	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_WE32KRAM, bbc_we32kram_device);



#endif /* MAME_BUS_BBC_INTERNAL_WE32KRAM_H */
