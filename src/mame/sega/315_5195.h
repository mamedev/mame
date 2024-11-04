// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 315-5195 memory mapper

***************************************************************************/

#ifndef MAME_SEGA_315_5195_H
#define MAME_SEGA_315_5195_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "fd1089.h"
#include "fd1094.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> sega_315_5195_mapper_device

class sega_315_5195_mapper_device : public device_t
{
public:
	typedef device_delegate<void (sega_315_5195_mapper_device &, u8)> mapper_delegate;

	// construction/destruction
	template <typename T>
	sega_315_5195_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: sega_315_5195_mapper_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_cpuregion.set_tag(std::forward<T>(cpu_tag));
	}

	sega_315_5195_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <typename... T> void set_mapper(T &&... args) { m_mapper.set(std::forward<T>(args)...); }

	auto pbf() { return m_pbf_callback.bind(); }
	auto mcu_int() { return m_mcu_int_callback.bind(); }

	// public interface
	u8 read(address_space &space, offs_t offset);
	void write(offs_t offset, u8 data);
	u8 pread();
	void pwrite(u8 data);

	// to use when the underlying decryption changes and remapping is needed
	void decryption_changed();

	// mapping helpers
	void map_as_rom(u32 offset, u32 length, offs_t mirror, const char *bank_name, const char *decrypted_bank_name, offs_t rgnoffset, write16_delegate whandler);
	void map_as_ram(u32 offset, u32 length, offs_t mirror, const char *bank_share_name, write16_delegate whandler);
	void map_as_region(u32 offset, u32 length, offs_t mirror, const char *region_name, write16_delegate whandler);
	void map_as_handler(u32 offset, u32 length, offs_t mirror, read16_delegate rhandler, write16_delegate whandler);

	// perform an explicit configuration (for bootlegs with hard-coded mappings)
	void configure_explicit(const u8 *map_data);

	// open bus read helpers
	u16 open_bus_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(write_to_sound);
	TIMER_CALLBACK_MEMBER(write_from_sound);

	// internal region struct
	struct region_info
	{
		offs_t  size_mask;
		offs_t  base;
		offs_t  mirror;
		offs_t  start;
		offs_t  end;
	};

	// helper class for tracking banks mapped to ROM regions
	class decrypt_bank
	{
	public:
		// construction/destruction
		decrypt_bank();
		~decrypt_bank();

		// configuration
		void set_decrypt(fd1089_base_device *fd1089);
		void set_decrypt(fd1094_device *fd1094);
		void clear() { m_start = 0; m_end = ~0; m_rgnoffs = 0; m_srcptr = nullptr; }
		void set(offs_t start, offs_t end, offs_t rgnoffs, void *src, memory_bank *bank);

		// updating
		void update(memory_bank *bank);
		void reset() { m_fd1089_decrypted.clear(); if (m_fd1094_cache != nullptr) m_fd1094_cache->reset(); }

	private:
		// internal state
		offs_t             m_start;
		offs_t             m_end;
		offs_t             m_rgnoffs;
		u8 *               m_srcptr;
		fd1089_base_device *m_fd1089;
		std::vector<u16>   m_fd1089_decrypted;
		std::unique_ptr<fd1094_decryption_cache> m_fd1094_cache;
	};

	// internal helpers
	void compute_region(region_info &info, u8 index, u32 length, u32 mirror, u32 offset = 0);
	void update_mapping();
	void fd1094_state_change(u8 state);

	// configuration
	required_device<m68000_device> m_cpu;
	required_memory_region      m_cpuregion;
	mapper_delegate             m_mapper;
	devcb_write_line            m_pbf_callback;
	devcb_write_line            m_mcu_int_callback;

	// internal state
	address_space *              m_space;
	address_space *              m_decrypted_space;
	memory_bank_array_creator<8> m_decryption_banks;
	u8                           m_regs[0x20];
	u8                           m_curregion;
	decrypt_bank                 m_banks[8];

	// communication registers
	u8                     m_to_sound;
	u8                     m_from_sound;

	// flag to track recursion through open_bus_r
	bool     m_open_bus_recurse;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5195_MEM_MAPPER, sega_315_5195_mapper_device)


#endif // MAME_SEGA_315_5195_H
