// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk Fourmeg RAM/ROM Expansions

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_4Meg.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_STL4M32_H
#define MAME_BUS_BBC_INTERNAL_STL4M32_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_stl4m32_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_stl4m32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(clock_switch);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual bool overrides_ram() override { return true; }
	virtual bool overrides_mos() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;
	virtual void romsel_w(offs_t offset, uint8_t data) override;
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mos_r(offs_t offset) override;
	virtual void mos_w(offs_t offset, uint8_t data) override;

	optional_device_array<bbc_romslot_device, 16> m_rom;

	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_romsel;
	uint8_t m_ramsel;
	uint8_t m_shadow;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_STL4M32, bbc_stl4m32_device);


#endif /* MAME_BUS_BBC_INTERNAL_STL4M32_H */
