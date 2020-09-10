// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

NEO-ZMC Memory controller

***************************************************************************/

#ifndef MAME_MACHINE_NEO_ZMC_H
#define MAME_MACHINE_NEO_ZMC_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neo_zmc_device

class neo_zmc_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configurations
	template <typename T> void set_default_rom_tag(T &&tag) { m_default_rom.set_tag(std::forward<T>(tag)); }

	// I/O operations
	u8 set_bank_r(offs_t offset);

	// ROM read handlers
	u8 rom_r(offs_t offset) { return m_rom_cache.read_byte(offset & 0x7ffff); } // 19 bit address

	template<unsigned Slot>
	u8 banked_rom_r(offs_t offset) { offset &= ((0x800 << Slot) - 1); return rom_r(m_bank[Slot] | offset); }

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_rom_space_config;

private:
	// configurations
	optional_region_ptr<u8> m_default_rom;

	// internal states
	memory_access<19, 0, 0, ENDIANNESS_LITTLE>::cache m_rom_cache;

	u32 m_bank[4];
};


// device type definition
DECLARE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_NEO_ZMC_H
