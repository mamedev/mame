// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Expansion v3 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_FE3_H
#define MAME_BUS_VIC20_FE3_H

#pragma once

#include "exp.h"
#include "machine/intelfsh.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_final_expansion_3_device

class vic20_final_expansion_3_device :  public device_t,
									public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_final_expansion_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	enum
	{
		REG1_BLK0      = 0x01,
		REG1_BLK1      = 0x02,
		REG1_BLK2      = 0x04,
		REG1_BLK3      = 0x08,
		REG1_BLK5      = 0x10,
		REG1_START     = 0x00,
		REG1_SUPER_ROM = 0x40,
		REG1_RAM_1     = 0x80,
		REG1_RAM_2     = 0xc0,
		REG1_SUPER_RAM = 0xa0,
		REG1_RAM_ROM   = 0x60,
		REG1_FLASH     = 0x20,
		REG1_MODE_MASK = 0xe0
	};

	enum
	{
		REG2_BLK0 = 0x01,
		REG2_BLK1 = 0x02,
		REG2_BLK2 = 0x04,
		REG2_BLK3 = 0x08,
		REG2_BLK5 = 0x10,
		REG2_A13  = 0x20,
		REG2_A14  = 0x40,
		REG2_IO3  = 0x80
	};

	offs_t get_address(int bank, int block, offs_t offset);
	uint8_t read_register(offs_t offset);
	void write_register(offs_t offset, uint8_t data);

	required_device<amd_29f040_device> m_flash_rom;
	memory_share_creator<uint8_t> m_ram;

	uint8_t m_reg1;
	uint8_t m_reg2;
	int m_lockbit;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_FE3, vic20_final_expansion_3_device)

#endif // MAME_BUS_VIC20_FE3_H
