// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    GCC (Cambridge) Romex 13

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/GCC_Romex13.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_ROMEX_H
#define MAME_BUS_BBC_INTERNAL_ROMEX_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_romex13_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_romex13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

private:
	optional_device_array<bbc_romslot_device, 16> m_rom;

	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ROMEX13, bbc_romex13_device);


#endif /* MAME_BUS_BBC_INTERNAL_ROMEX_H */
