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

class vic20_final_expansion_3_device : public device_t,
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

	enum
	{
		TARGET_NONE,
		TARGET_RAM,
		TARGET_FLASH
	};

	required_device<amd_29f040_device> m_flash_rom;
	memory_share_creator<uint8_t> m_ram;

	offs_t get_address(int bank, int block, offs_t offset);
	uint8_t read_register(offs_t offset);
	void write_register(offs_t offset, uint8_t data);

	std::pair<int, int> read_target(int block) const;
	std::pair<int, int> write_target(int block) const;
	void set_lockbit(int state);
	void update_map();
	void update_loram(vic20_expansion_window &window, offs_t offset);
	void update_block(vic20_expansion_window &window, int block);
	void update_blk5();
	void update_io3();
	uint8_t blk5_r(offs_t offset);
	void blk5_w(offs_t offset, uint8_t data);

	uint8_t m_reg1;
	uint8_t m_reg2;
	int m_lockbit;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_FE3, vic20_final_expansion_3_device)

#endif // MAME_BUS_VIC20_FE3_H
