// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KC82 CPU core with MMU

***************************************************************************/

#ifndef MAME_CPU_Z80_KC82_H
#define MAME_CPU_Z80_KC82_H

#pragma once

#include "z80.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kc82_device : public z80_device
{
public:
	enum
	{
		KC82_B1 = Z80_WZ + 1, KC82_B2, KC82_B3, KC82_B4,
		KC82_A1, KC82_A2, KC82_A3
	};

protected:
	// construction/destruction
	kc82_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor io_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// z80_device overrides
	virtual u8 data_read(u16 addr) override;
	virtual void data_write(u16 addr, u8 value) override;
	virtual u8 opcode_read() override;
	virtual u8 arg_read() override;

	// MMU access
	u8 mmu_r(offs_t offset);
	void mmu_w(offs_t offset, u8 data);

private:
	// internal helpers
	void mmu_remap_pages();

	// address spaces
	address_space_config m_program_config;
	address_space_config m_opcodes_config;
	address_space_config m_io_config;

	// MMU registers
	u16 m_mmu_a[4];
	u8 m_mmu_b[5];
	u32 m_mmu_base[0x40];
};


#endif // MAME_CPU_Z80_KC82_H
