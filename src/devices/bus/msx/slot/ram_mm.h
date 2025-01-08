// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_MM_H
#define MAME_BUS_MSX_SLOT_RAM_MM_H

#pragma once

#include "slot.h"

class msx_slot_ram_mm_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_ram_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	msx_slot_ram_mm_device &set_total_size(u32 total_size) { m_total_size = total_size; return *this; }
	msx_slot_ram_mm_device &set_unused_bits(u8 unused_bits) { m_unused_bits = unused_bits; return *this; }

	// Backdoor for the Turbo-R firmware/internal mapper to access internal RAM.
	u32 get_ram_size() { return m_total_size; }
	u8 *get_ram_base() { return &m_ram[0]; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u8 read_mapper_bank(offs_t offset);
	void write_mapper_bank(offs_t offset, u8 data);

	std::unique_ptr<u8[]> m_ram;
	u32 m_total_size;
	u8 m_bank_mask;
	u8 m_unused_bits;
	memory_bank_array_creator<4> m_rambank;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_RAM_MM, msx_slot_ram_mm_device)

#endif // MAME_BUS_MSX_SLOT_RAM_MM_H
