// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#ifndef MAME_MACHINE_SEGAIC16_H
#define MAME_MACHINE_SEGAIC16_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/fd1089.h"
#include "machine/fd1094.h"
#include "emupal.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> sega_16bit_common_base

class sega_16bit_common_base : public driver_device
{
public:
	// open bus read helpers
	u16 open_bus_r(address_space &space);

	// palette helpers
	DECLARE_WRITE16_MEMBER( paletteram_w );
	DECLARE_WRITE16_MEMBER( hangon_paletteram_w );
	DECLARE_WRITE16_MEMBER( philko_paletteram_w );

protected:
	// construction/destruction
	sega_16bit_common_base(const machine_config &mconfig, device_type type, const char *tag);

	// internal helpers
	void palette_init();

public: // -- stupid system16.cpp
	// memory pointers
	required_shared_ptr<u16> m_paletteram;
protected:

	// internal state
	bool     m_open_bus_recurse;         // flag to track recursion through open_bus_r
	u32      m_palette_entries;          // number of palette entries
	u8       m_palette_normal[32];       // RGB translations for normal pixels
	u8       m_palette_shadow[32];       // RGB translations for shadowed pixels
	u8       m_palette_hilight[32];      // RGB translations for hilighted pixels
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


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
	template <typename... T> void set_mapper(T &&... args) { m_mapper = mapper_delegate(std::forward<T>(args)...); }

	auto pbf() { return m_pbf_callback.bind(); }
	auto mcu_int() { return m_mcu_int_callback.bind(); }

	// public interface
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( pread );
	DECLARE_WRITE8_MEMBER( pwrite );

	// mapping helpers
	void map_as_rom(u32 offset, u32 length, offs_t mirror, const char *bank_name, const char *decrypted_bank_name, offs_t rgnoffset, write16_delegate whandler);
	void map_as_ram(u32 offset, u32 length, offs_t mirror, const char *bank_share_name, write16_delegate whandler);
	void map_as_handler(u32 offset, u32 length, offs_t mirror, read16_delegate rhandler, write16_delegate whandler);

	// perform an explicit configuration (for bootlegs with hard-coded mappings)
	void configure_explicit(const u8 *map_data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
		void clear() { set(nullptr, nullptr, 0, 0, ~0, nullptr); }
		void set(memory_bank *bank, memory_bank *decrypted_bank, offs_t start, offs_t end, offs_t rgnoffs, u8 *src);

		// updating
		void update();
		void reset() { m_fd1089_decrypted.clear(); if (m_fd1094_cache != nullptr) m_fd1094_cache->reset(); }

	private:
		// internal state
		memory_bank *      m_bank;
		memory_bank *      m_decrypted_bank;
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
	address_space *        m_space;
	address_space *        m_decrypted_space;
	u8                     m_regs[0x20];
	u8                     m_curregion;
	decrypt_bank           m_banks[8];

	// communication registers
	u8                     m_to_sound;
	u8                     m_from_sound;
};


// ======================> sega_315_5248_multiplier_device

class sega_315_5248_multiplier_device : public device_t
{
public:
	// construction/destruction
	sega_315_5248_multiplier_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// public interface
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	u16                      m_regs[4];
};


// ======================> sega_315_5249_divider_device

class sega_315_5249_divider_device : public device_t
{
public:
	// construction/destruction
	sega_315_5249_divider_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// public interface
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal helpers
	void execute(int mode);

	// internal state
	u16                      m_regs[8];
};


// ======================> sega_315_5250_compare_timer_device

class sega_315_5250_compare_timer_device : public device_t
{
public:
	// construction/destruction
	sega_315_5250_compare_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto m68kint_callback() { return m_68kint_callback.bind(); }
	auto zint_callback() { return m_zint_callback.bind(); }

	// public interface
	DECLARE_WRITE_LINE_MEMBER(exck_w);
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 zread();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal helpers
	void execute(bool update_history = false);
	void interrupt_ack();
	TIMER_CALLBACK_MEMBER(write_to_sound);

	// configuration
	devcb_write_line         m_68kint_callback;
	devcb_write_line         m_zint_callback;

	// internal state
	u16                      m_regs[16];
	u16                      m_counter;
	u8                       m_bit;
	bool                     m_exck;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5195_MEM_MAPPER,    sega_315_5195_mapper_device)
DECLARE_DEVICE_TYPE(SEGA_315_5248_MULTIPLIER,    sega_315_5248_multiplier_device)
DECLARE_DEVICE_TYPE(SEGA_315_5249_DIVIDER,       sega_315_5249_divider_device)
DECLARE_DEVICE_TYPE(SEGA_315_5250_COMPARE_TIMER, sega_315_5250_compare_timer_device)


#endif // MAME_MACHINE_SEGAIC16_H
