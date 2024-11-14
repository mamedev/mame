// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computech Integra-B expansion board

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_INTEGRAB_H
#define MAME_BUS_BBC_INTERNAL_INTEGRAB_H

#include "internal.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_integrab_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_integrab_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual bool overrides_ram() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;
	virtual uint8_t romsel_r(offs_t offset) override;
	virtual void romsel_w(offs_t offset, uint8_t data) override;
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_ibos;
	optional_device_array<bbc_romslot_device, 16> m_rom;
	required_device<nvram_device> m_nvram;
	required_device<mc146818_device> m_rtc;
	required_ioport m_wp;

	std::unique_ptr<uint8_t[]> m_ram;
	bool m_shadow;
	uint8_t m_romsel;
	uint8_t m_ramsel;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_INTEGRAB, bbc_integrab_device)


#endif // MAME_BUS_BBC_INTERNAL_INTEGRAB_H
