// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Expansion v3 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __VIC20_FE3__
#define __VIC20_FE3__

#include "emu.h"
#include "exp.h"
#include "machine/intelfsh.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_final_expansion_3_t

class vic20_final_expansion_3_t :  public device_t,
									public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_final_expansion_3_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

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
	UINT8 read_register(offs_t offset);
	void write_register(offs_t offset, UINT8 data);

	required_device<amd_29f040_device> m_flash_rom;
	optional_shared_ptr<UINT8> m_ram;

	UINT8 m_reg1;
	UINT8 m_reg2;
	int m_lockbit;
};


// device type definition
extern const device_type VIC20_FE3;



#endif
