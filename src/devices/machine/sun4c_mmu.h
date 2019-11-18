// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sun4c_mmu.h - Sun 4c MMU emulation

***************************************************************************/

#ifndef MAME_MACHINE_SUN4C_MMU_H
#define MAME_MACHINE_SUN4C_MMU_H

#pragma once

#include "cpu/sparc/sparc_intf.h"
#include "machine/ram.h"
#include "machine/z80scc.h"

class sun4c_mmu_device : public device_t, public sparc_mmu_interface
{
public:
	sun4c_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint8_t ctx_mask, uint8_t pmeg_mask)
		: sun4c_mmu_device(mconfig, tag, owner, clock)
	{
		set_ctx_mask(ctx_mask);
		set_pmeg_mask(pmeg_mask);
	}

	sun4c_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&cpu_tag) { m_cpu.set_tag(std::forward<T>(cpu_tag)); }
	template <typename T> void set_ram(T &&ram_tag) { m_ram.set_tag(std::forward<T>(ram_tag)); }
	template <typename T> void set_rom(T &&rom_tag) { m_rom.set_tag(std::forward<T>(rom_tag)); }
	template <typename T> void set_scc(T &&scc_tag) { m_scc.set_tag(std::forward<T>(scc_tag)); }

	auto type1_r() { return m_type1_r.bind(); }
	auto type1_w() { return m_type1_w.bind(); }

	void set_ctx_mask(uint8_t ctx_mask) { m_ctx_mask = ctx_mask; }
	void set_pmeg_mask(uint8_t pmeg_mask) { m_pmeg_mask = pmeg_mask; }

	enum insn_data_mode
	{
		USER_INSN,
		SUPER_INSN,
		USER_DATA,
		SUPER_DATA
	};

	template <insn_data_mode MODE> uint32_t insn_data_r(const uint32_t offset, const uint32_t mem_mask);
	template <insn_data_mode MODE> void insn_data_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask);

	// sparc_mmu_device overrides
	uint32_t fetch_insn(const bool supervisor, const uint32_t offset) override;
	uint32_t read_asi(uint8_t asi, uint32_t offset, uint32_t mem_mask) override;
	void write_asi(uint8_t asi, uint32_t offset, uint32_t data, uint32_t mem_mask) override;
	void set_host(sparc_mmu_host_interface *host) override { m_host = host; }

protected:
	static const device_timer_id TIMER_RESET = 0;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint32_t cache_flush_r();
	void cache_flush_w();
	uint32_t system_r(const uint32_t offset, const uint32_t mem_mask);
	void system_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask);
	uint32_t segment_map_r(const uint32_t offset, const uint32_t mem_mask);
	void segment_map_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask);
	uint32_t page_map_r(const uint32_t offset, const uint32_t mem_mask);
	void page_map_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask);
	void type0_timeout_r(const uint32_t offset);
	void type0_timeout_w(const uint32_t offset);

	enum
	{
		// system enable constants
		ENA_NOTBOOT     = 0x80,
		ENA_SDVMA       = 0x20,
		ENA_CACHE       = 0x10,
		ENA_RESET       = 0x04,
		ENA_DIAG        = 0x01,

		// page table entry constants
		PM_VALID        = 0x80000000,    // page is valid
		PM_WRITEMASK    = 0x40000000,    // writable?
		PM_SYSMASK      = 0x20000000,    // system use only?
		PM_CACHE        = 0x10000000,    // cachable?
		PM_TYPEMASK     = 0x0c000000,    // type mask
		PM_ACCESSED     = 0x02000000,    // accessed flag
		PM_MODIFIED     = 0x01000000     // modified flag
	};

	struct page_entry_t
	{
		uint32_t valid;
		uint32_t writable;
		uint32_t supervisor;
		uint32_t uncached;
		uint32_t accessed;
		uint32_t modified;
		uint32_t page;
		uint8_t type;
		uint8_t pad[3];

		uint32_t to_uint();
		void merge_uint(uint32_t data, uint32_t mem_mask);
	};

	required_device<cpu_device> m_cpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<z80scc_device> m_scc;

	sparc_mmu_host_interface *m_host;

	// Bus access callbacks
	devcb_read32 m_type1_r;
	devcb_write32 m_type1_w;

	// Actual SRAM
	uint8_t m_segmap[16][4096];
	page_entry_t m_pagemap[16384];
	uint32_t m_cachetags[16384];
	uint32_t m_cachedata[16384];

	uint32_t *m_rom_ptr;
	uint32_t *m_ram_ptr;
	uint32_t m_ram_size;
	uint32_t m_ram_size_words;
	uint32_t m_context;
	uint32_t m_context_masked;
	uint32_t m_cache_context;
	uint8_t m_system_enable;
	bool m_fetch_bootrom;
	uint32_t m_buserr[4];

	// Pre-computed data for optimization purposes
	uint32_t m_segmap_masked[16][4096];
	uint8_t *m_curr_segmap;
	uint32_t *m_curr_segmap_masked;
	bool m_page_valid[16384];

	// Internal MAME device state
	uint8_t m_ctx_mask;         // SS2 is sun4c but has 16 contexts; most have 8
	uint8_t m_pmeg_mask;        // SS2 is sun4c but has 16384 PTEs; most have 8192
	uint32_t m_ram_set_mask[4]; // Used for mirroring within 4 megabyte sets
	uint32_t m_ram_set_base[4];
	uint32_t m_populated_ram_words;
	emu_timer *m_reset_timer;
};


DECLARE_DEVICE_TYPE(SUN4C_MMU, sun4c_mmu_device)

#endif // MAME_MACHINE_SUN4C_MMU_H
