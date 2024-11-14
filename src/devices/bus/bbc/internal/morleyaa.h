// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics 'AA' Master ROM Expansion Board

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_MORLEY_H
#define MAME_BUS_BBC_INTERNAL_MORLEY_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_morleyaa_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_morleyaa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_aa_rom;
	required_device_array<bbc_romslot_device, 12> m_rom;

	uint8_t m_romsel;
	uint8_t m_banksel;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MORLEYAA, bbc_morleyaa_device);


#endif /* MAME_BUS_BBC_INTERNAL_MORLEY_H */
