// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Multiscreen cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_MULTISCREEN_H
#define MAME_BUS_C64_MULTISCREEN_H

#pragma once


#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_multiscreen_cartridge_device

class c64_multiscreen_cartridge_device : public device_t,
											public device_c64_expansion_card_interface,
											public device_nvram_interface
{
public:
	// construction/destruction
	c64_multiscreen_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_memory_region m_eprom;

	uint8_t m_bank;

	void multiscreen_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_MULTISCREEN, c64_multiscreen_cartridge_device)


#endif // MAME_BUS_C64_MULTISCREEN_H
