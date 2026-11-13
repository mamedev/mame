// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Vic Flash Plugin cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VFP_H
#define MAME_BUS_VIC20_VFP_H

#pragma once

#include "exp.h"
#include "machine/intelfsh.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_vic_flash_plugin_device

class vic20_vic_flash_plugin_device :  public device_t,
										public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_vic_flash_plugin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	offs_t flash_address(offs_t offset) const;
	uint8_t blk5_r(offs_t offset);
	void blk5_w(offs_t offset, uint8_t data);
	uint8_t io2_r(offs_t offset);
	void io2_w(offs_t offset, uint8_t data);
	void update_map();

	required_device<amd_29f032_device> m_flash;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_ram;

	uint8_t m_bank;
	uint8_t m_cfg;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_VFP, vic20_vic_flash_plugin_device)

#endif // MAME_BUS_VIC20_VFP_H
