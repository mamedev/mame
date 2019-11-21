// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sun4c_mmu.cpp - Sun 4/4c MMU emulation

***************************************************************************/

#include "emu.h"
#include "sun4c_mmu.h"

#include "cpu/sparc/sparc.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(SUN4_MMU, sun4_mmu_device, "sun4_mmu", "Sun 4 MMU")
DEFINE_DEVICE_TYPE(SUN4C_MMU, sun4c_mmu_device, "sun4c_mmu", "Sun 4c MMU")

#define LOG_PAGE_MAP        (1U << 0)
#define LOG_SEGMENT_MAP     (1U << 1)
#define LOG_INVALID_PTE     (1U << 2)
#define LOG_SYSTEM          (1U << 3)
#define LOG_CONTEXT         (1U << 4)
#define LOG_SYSTEM_ENABLE   (1U << 5)
#define LOG_BUSERROR        (1U << 6)
#define LOG_CACHE_TAGS      (1U << 7)
#define LOG_CACHE_DATA      (1U << 8)
#define LOG_UNKNOWN_SYSTEM  (1U << 9)
#define LOG_UNKNOWN_SEGMENT (1U << 10)
#define LOG_TYPE0_TIMEOUT   (1U << 11)
#define LOG_UNKNOWN_SPACE   (1U << 12)
#define LOG_WRITE_PROTECT   (1U << 13)
#define LOG_ALL_ASI         (1U << 14) // WARNING: Heavy!

#define VERBOSE (0)
#include "logmacro.h"

sun4_mmu_base_device::sun4_mmu_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_scc(*this, finder_base::DUMMY_TAG)
	, m_host(nullptr)
	, m_type1_r(*this)
	, m_type1_w(*this)
	, m_rom_ptr(nullptr)
	, m_ram_ptr(nullptr)
	, m_ram_size(0)
	, m_ram_size_words(0)
	, m_context(0)
	, m_context_masked(0)
	, m_system_enable(0)
	, m_fetch_bootrom(true)
	, m_curr_segmap(nullptr)
	, m_curr_segmap_masked(nullptr)
{
}

sun4_mmu_device::sun4_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sun4_mmu_base_device(mconfig, SUN4_MMU, tag, owner, clock, 7, 0x7f, 0x7ff, 11, 0x1f, 0x7ffff, 0xfff)
{
}

sun4c_mmu_device::sun4c_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sun4_mmu_base_device(mconfig, SUN4C_MMU, tag, owner, clock, 7, 0x7f, 0x3ff, 10, 0x3f, 0xffff, 0x3fff)
{
}

void sun4_mmu_base_device::device_start()
{
	m_type1_r.resolve_safe(0xffffffff);
	m_type1_w.resolve_safe();

	// allocate timer for system reset
	m_reset_timer = timer_alloc(TIMER_RESET);
	m_reset_timer->adjust(attotime::never);

	m_segmap = std::make_unique<std::unique_ptr<uint8_t[]>[]>(16);
	m_segmap_masked = std::make_unique<std::unique_ptr<uint32_t[]>[]>(16);

	for (int i = 0; i < 16; i++)
	{
		m_segmap[i] = std::make_unique<uint8_t[]>(16384);
		m_segmap_masked[i] = std::make_unique<uint32_t[]>(16384);
		save_pointer(NAME(m_segmap[i]), 16384, i);
		save_pointer(NAME(m_segmap_masked[i]), 16384, i);
	}

	m_pagemap = std::make_unique<page_entry[]>(16384);
	save_pointer(NAME(reinterpret_cast<uint8_t*>(m_pagemap.get())), sizeof(page_entry) * 16384);

	m_cachetags = std::make_unique<uint32_t[]>(16384);
	save_pointer(NAME(m_cachetags), 16384);

	m_cachedata = std::make_unique<uint32_t[]>(16384);
	save_pointer(NAME(m_cachedata), 16384);

	m_page_valid = std::make_unique<bool[]>(16384);
	save_pointer(NAME(m_page_valid), 16384);

	save_item(NAME(m_ram_size));
	save_item(NAME(m_ram_size_words));
	save_item(NAME(m_context));
	save_item(NAME(m_context_masked));
	save_item(NAME(m_system_enable));
	save_item(NAME(m_fetch_bootrom));
	save_item(NAME(m_buserr));
	save_item(NAME(m_ctx_mask));
	save_item(NAME(m_pmeg_mask));
	save_item(NAME(m_page_mask));
	save_item(NAME(m_seg_entry_shift));
	save_item(NAME(m_seg_entry_mask));
	save_item(NAME(m_page_entry_mask));
	save_item(NAME(m_cache_mask));
	save_item(NAME(m_ram_set_mask));
	save_item(NAME(m_ram_set_base));
	save_item(NAME(m_populated_ram_words));

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("l2p", CMDFLAG_NONE, 0, 1, 1, std::bind(&sun4_mmu_base_device::l2p_command, this, _1, _2));
	}
}

void sun4_mmu_base_device::device_reset()
{
	m_rom_ptr = (uint32_t *)m_rom->base();
	m_ram_ptr = (uint32_t *)m_ram->pointer();
	m_ram_size = m_ram->size();
	m_ram_size_words = m_ram_size >> 2;
	const uint32_t num_16meg_sets = m_ram_size / 0x1000000;
	const uint32_t leftover_4meg_size = m_ram_size % 0x1000000;
	const uint32_t num_4meg_sets = leftover_4meg_size / 0x400000;
	uint32_t base = 0;
	uint32_t set = 0;
	for (; set < num_16meg_sets; set++)
	{
		m_ram_set_base[set] = base;
		m_ram_set_mask[set] = 0x003fffff;
		base += 0x1000000 >> 2;
	}
	for (; set < num_16meg_sets+num_4meg_sets; set++)
	{
		m_ram_set_base[set] = base;
		m_ram_set_mask[set] = 0x000fffff;
		base += 0x400000 >> 2;
	}
	for (; set < 4; set++)
	{
		m_ram_set_mask[set] = 0;
		m_ram_set_base[set] = 0;
	}
	m_populated_ram_words = (num_16meg_sets + num_4meg_sets) * (0x1000000 >> 2);

	m_context = 0;
	m_context_masked = 0;
	m_curr_segmap = &m_segmap[0][0];
	m_curr_segmap_masked = &m_segmap_masked[0][0];
	m_system_enable = 0;
	m_fetch_bootrom = true;

	memset(m_buserr, 0, sizeof(uint32_t) * 4);
	for (int i = 0; i < 16; i++)
	{
		memset(&m_segmap[i][0], 0, 4096);
	}
	memset(&m_pagemap[0], 0, sizeof(page_entry) * 16384);
	memset(&m_cachetags[0], 0, sizeof(uint32_t) * 16384);
	memset(&m_cachedata[0], 0, sizeof(uint32_t) * 16384);
}

void sun4_mmu_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_RESET)
	{
		m_reset_timer->adjust(attotime::never);
		m_cpu->set_input_line(SPARC_RESET, CLEAR_LINE);
	}
}

uint32_t sun4_mmu_base_device::fetch_insn(const bool supervisor, const uint32_t offset)
{
	if (supervisor)
		return insn_data_r<SUPER_INSN>(offset, 0xffffffff);
	else
		return insn_data_r<USER_INSN>(offset, 0xffffffff);
}

uint32_t sun4_mmu_base_device::read_asi(uint8_t asi, uint32_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_ALL_ASI, "read_asi %d: %08x & %08x\n", asi, offset << 2, mem_mask);
	switch (asi)
	{
		case 2:
			return system_r(offset, mem_mask);
		case 3:
			return segment_map_r(offset, mem_mask);
		case 4:
			return page_map_r(offset, mem_mask);
		case 8:
			return insn_data_r<USER_INSN>(offset, mem_mask);
		case 9:
			return insn_data_r<SUPER_INSN>(offset, mem_mask);
		case 10:
			return insn_data_r<USER_DATA>(offset, mem_mask);
		case 11:
			return insn_data_r<SUPER_DATA>(offset, mem_mask);
		case 12:
		case 13:
		case 14:
			cache_flush_r();
			return 0;
		default:
			return ~0;
	}
}

void sun4_mmu_base_device::write_asi(uint8_t asi, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ALL_ASI, "write_asi %d: %08x = %08x & %08x\n", asi, offset << 2, data, mem_mask);
	switch (asi)
	{
		case 2:
			system_w(offset, data, mem_mask);
			return;
		case 3:
			segment_map_w(offset, data, mem_mask);
			return;
		case 4:
			page_map_w(offset, data, mem_mask);
			return;
		case 8:
			insn_data_w<USER_INSN>(offset, data, mem_mask);
			return;
		case 9:
			insn_data_w<SUPER_INSN>(offset, data, mem_mask);
			return;
		case 10:
			insn_data_w<USER_DATA>(offset, data, mem_mask);
			return;
		case 11:
			insn_data_w<SUPER_DATA>(offset, data, mem_mask);
			return;
		case 12:
		case 13:
		case 14:
			cache_flush_w();
			return;
		default:
			return;
	}
}

uint32_t sun4_mmu_base_device::cache_flush_r()
{
	// Do nothing for now
	return 0;
}

void sun4_mmu_base_device::cache_flush_w()
{
	// Do nothing for now
}

uint32_t sun4_mmu_base_device::system_r(const uint32_t offset, const uint32_t mem_mask)
{
	LOGMASKED(LOG_SYSTEM, "%s: system_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	switch (offset >> 26)
	{
		case 3: // context reg
		{
			if (mem_mask == 0x00ff0000)
			{
				LOGMASKED(LOG_CONTEXT, "sun4c_mmu: read context %08x & %08x = %08x\n", offset << 2, mem_mask, m_context<<16);
				return m_context<<16;
			}
			LOGMASKED(LOG_CONTEXT, "sun4c_mmu: read context %08x & %08x = %08x\n", offset << 2, mem_mask, m_context<<24);
			return m_context<<24;
		}

		case 4: // system enable reg
			LOGMASKED(LOG_SYSTEM_ENABLE, "sun4c_mmu: read system enable %08x & %08x = %08x\n", offset << 2, mem_mask, m_system_enable<<24);
			return m_system_enable<<24;

		case 6: // bus error register
		{
			const uint32_t ret = m_buserr[offset & 0xf];
			LOGMASKED(LOG_BUSERROR, "sun4c_mmu: read buserror %08x & %08x = %08x, PC=%x\n", 0x60000000 | (offset << 2), mem_mask, ret, m_cpu->pc());
			m_buserr[offset & 0xf] = 0; // clear on reading
			return ret;
		}

		case 8: // (d-)cache tags
			LOGMASKED(LOG_CACHE_TAGS, "sun4_mmu: read dcache tags @ %x, PC = %x\n", offset, m_cpu->pc());
			return m_cachetags[offset & m_cache_mask];

		case 9: // (d-)cache data
			LOGMASKED(LOG_CACHE_DATA, "sun4c_mmu: read dcache data @ %x, PC = %x\n", offset, m_cpu->pc());
			return m_cachedata[offset & m_cache_mask];

		case 0xf:   // UART bypass
			switch (offset & 3)
			{
				case 0: if (mem_mask == 0xff000000) return m_scc->cb_r(0)<<24; else return m_scc->db_r(0)<<8; break;
				case 1: if (mem_mask == 0xff000000) return m_scc->ca_r(0)<<24; else return m_scc->da_r(0)<<8; break;
			}
			return 0xffffffff;

		case 0: // IDPROM - SPARCstation-1 does not have an ID prom and a timeout should occur.
		default:
			LOGMASKED(LOG_UNKNOWN_SYSTEM, "read unhandled ASI 2 space %08x & %08x\n", offset << 2, mem_mask);
			return 0;
	}
}

void sun4_mmu_base_device::system_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask)
{
	LOGMASKED(LOG_SYSTEM, "system_w: %08x = %08x & %08x\n", offset << 2, data, mem_mask);
	switch (offset >> 26)
	{
		case 3: // context reg
			LOGMASKED(LOG_CONTEXT, "write context = %08x & %08x\n", data, mem_mask);
			m_context = data >> 24;
			m_context_masked = m_context & m_ctx_mask;
			m_cache_context = m_context & m_ctx_mask;
			m_curr_segmap = &m_segmap[m_context_masked][0];
			m_curr_segmap_masked = &m_segmap_masked[m_context_masked][0];
			return;

		case 4: // system enable reg
		{
			LOGMASKED(LOG_SYSTEM_ENABLE, "write system enable = %08x & %08x\n", data, mem_mask);
			m_system_enable = data >> 24;
			m_fetch_bootrom = !(m_system_enable & ENA_NOTBOOT);

			if (m_system_enable & ENA_RESET)
			{
				m_reset_timer->adjust(attotime::from_usec(1));
				m_cpu->set_input_line(SPARC_RESET, ASSERT_LINE);
			}
			if (m_system_enable & ENA_RESET)
			{
				m_system_enable = 0;
				m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			return;
		}

		case 6: // bus error
		{
			const uint32_t masked_offset = offset & 0xf;
			LOGMASKED(LOG_BUSERROR, "write bus error %08x = %08x & %08x\n", offset << 2, data, mem_mask);
			if (masked_offset == 0)
				m_buserr[0] = (data & 0x000000ff) | 0x00008000;
			else if (masked_offset == 1)
				m_buserr[1] = data;
			else if (masked_offset == 2)
				m_buserr[2] = data & 0x000000b0;
			else if (masked_offset == 3)
				m_buserr[3] = (data & 0x3fffffff) | ((data & 0x20000000) << 1) | ((data & 0x20000000) << 2);
			return;
		}

		case 8: // cache tags
			LOGMASKED(LOG_CACHE_TAGS, "write cache tags %08x = %08x & %08x\n", offset << 2, data, mem_mask);
			m_cachetags[offset & m_cache_mask] = data & 0x03f8fffc;
			return;

		case 9: // cache data
			LOGMASKED(LOG_CACHE_DATA, "write cache data %08x = %08x & %08x\n", offset << 2, data, mem_mask);
			m_cachedata[offset & m_cache_mask] = data | (1 << 19);
			return;

		case 0xf:   // UART bypass
			switch (offset & 3)
			{
				case 0: if (mem_mask == 0xff000000) m_scc->cb_w(0, data>>24); else m_scc->db_w(0, data>>8); break;
				case 1: if (mem_mask == 0xff000000) m_scc->ca_w(0, data>>24); else { m_scc->da_w(0, data>>8); printf("%c", data>>8); } break;
			}
			return;

		case 0: // IDPROM
		default:
			LOGMASKED(LOG_UNKNOWN_SYSTEM, "write unhandled ASI 2 space %08x = %08x & %08x, PC=%08x\n", offset << 2, data, mem_mask, m_cpu->pc());
			return;
	}
}

uint32_t sun4_mmu_base_device::segment_map_r(const uint32_t offset, const uint32_t mem_mask)
{
	uint32_t ret = 0;
	if (mem_mask == 0xffff0000)
		ret = m_curr_segmap[(offset>>16) & 0xfff]<<16;
	else if (mem_mask == 0xff000000)
		ret = m_curr_segmap[(offset>>16) & 0xfff]<<24;
	else if (mem_mask == 0xffffffff)
		ret = m_curr_segmap[(offset>>16) & 0xfff];
	else
		LOGMASKED(LOG_UNKNOWN_SEGMENT, "read segment map w/ unknown mask %08x & %08x\n", offset << 2, mem_mask);
	LOGMASKED(LOG_SEGMENT_MAP, "read segment map %08x & %08x = %08x\n", offset << 2, mem_mask, ret);
	return ret;
}

void sun4_mmu_base_device::segment_map_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask)
{
	LOGMASKED(LOG_SEGMENT_MAP, "write segment map %08x = %08x & %08x\n", offset << 2, data, mem_mask);

	uint8_t segdata = 0;
	if (mem_mask == 0xffff0000) segdata = (data >> 16) & 0xff;
	else if (mem_mask == 0xff000000) segdata = (data >> 24) & 0xff;
	else if (mem_mask == 0xffffffff) segdata = data & 0xff;
	else LOGMASKED(LOG_UNKNOWN_SEGMENT, "write segment map w/ unknown mask %08x = %08x & %08x, PC=%08x\n", offset << 2, data, mem_mask, m_cpu->pc());

	const uint32_t seg = (offset>>16) & 0xfff;
	m_curr_segmap[seg] = segdata;
	m_curr_segmap_masked[seg] = (segdata & m_pmeg_mask) << 6;
}

uint32_t sun4_mmu_base_device::page_map_r(const uint32_t offset, const uint32_t mem_mask)
{
	const uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	const uint32_t ret = page_entry_to_uint(page);
	LOGMASKED(LOG_PAGE_MAP, "read page map %08x & %08x (%x) = %08x\n", offset << 2, mem_mask, page, ret);
	return ret;
}

void sun4_mmu_base_device::page_map_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask)
{
	uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	LOGMASKED(LOG_PAGE_MAP, "write page map %08x (%x) = %08x & %08x\n", offset << 2, page, data, mem_mask);
	merge_page_entry(page, data, mem_mask);
	m_page_valid[page] = m_pagemap[page].valid;
}

void sun4_mmu_base_device::type0_timeout_r(const uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "type 0 read timeout %08x, PC=%08x\n", offset << 2, m_cpu->pc());
	m_buserr[0] = 0x20; // read timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

void sun4_mmu_base_device::type0_timeout_w(const uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "type 0 write timeout %08x, PC=%08x\n", offset << 2, m_cpu->pc());
	m_buserr[0] = 0x8020; // write timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

uint32_t sun4_mmu_base_device::page_entry_to_uint(uint32_t index)
{
	const page_entry &pe = m_pagemap[index];
	return pe.valid | pe.writable | pe.supervisor | pe.uncached | (pe.type << 26) | pe.accessed | pe.modified | (pe.page >> m_seg_entry_shift);
}

void sun4_mmu_base_device::merge_page_entry(uint32_t index, uint32_t data, uint32_t mem_mask)
{
	page_entry &pe = m_pagemap[index];
	const uint32_t new_value = (page_entry_to_uint(index) & ~mem_mask) | (data & mem_mask);
	pe.valid = new_value & PM_VALID;
	pe.writable = new_value & PM_WRITEMASK;
	pe.supervisor = new_value & PM_SYSMASK;
	pe.uncached = new_value & PM_CACHE;
	pe.type = (new_value & PM_TYPEMASK) >> 26;
	pe.accessed = new_value & PM_ACCESSED;
	pe.modified = new_value & PM_MODIFIED;
	pe.page = (new_value & m_page_entry_mask) << m_seg_entry_shift;
}

template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::USER_INSN>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::SUPER_INSN>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::USER_DATA>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::SUPER_DATA>(const uint32_t, const uint32_t);

template <sun4_mmu_base_device::insn_data_mode MODE>
uint32_t sun4_mmu_base_device::insn_data_r(const uint32_t offset, const uint32_t mem_mask)
{
	// supervisor program fetches in boot state are special
	if (MODE == SUPER_INSN && m_fetch_bootrom)
	{
		return m_rom_ptr[offset & 0x1ffff];
	}

	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];// & m_pmeg_mask;
	const uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);

	if (m_page_valid[entry_index])
	{
		page_entry &entry = m_pagemap[entry_index];
		entry.accessed = PM_ACCESSED;

		const uint32_t tmp = entry.page | (offset & m_page_mask);

		switch (entry.type)
		{
		case 0: // type 0 space
			if (tmp < m_populated_ram_words)
			{
				const uint32_t set = (tmp >> 22) & 3;
				const uint32_t addr_mask = m_ram_set_mask[set];
				const uint32_t masked_addr = m_ram_set_base[set] + (tmp & addr_mask);
				return m_ram_ptr[masked_addr];
			}
			else if (tmp >= 0x4000000 >> 2 && tmp < 0x10000000 >> 2)
			{
				type0_timeout_r(tmp);
			}
			return ~0;

		case 1: // type 1 space
			return m_type1_r(tmp, mem_mask);

		default:
			LOGMASKED(LOG_UNKNOWN_SPACE, "read unknown space type %d, %08x & %08x, PC=%08x\n", entry.type, tmp << 2, mem_mask, m_cpu->pc());
			m_host->set_mae();
			m_buserr[0] = 0x20;
			m_buserr[1] = offset << 2;
			return 0;
		}
	}
	else
	{
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_INVALID_PTE, "read invalid PTE %d (%08x), %08x & %08x, PC=%08x\n", entry_index, page_entry_to_uint(entry_index), offset << 2, mem_mask, m_cpu->pc());
			m_host->set_mae();
			m_buserr[0] |= 0x80;    // invalid PTE
			m_buserr[0] &= ~0x8000; // read
			m_buserr[1] = offset << 2;
			if (mem_mask != ~0 && mem_mask != 0xffff0000 && mem_mask != 0xff000000)
			{
				if (mem_mask == 0x0000ffff || mem_mask == 0x0000ff00)
				{
					m_buserr[1] |= 2;
				}
				else if (mem_mask == 0x00ff0000)
				{
					m_buserr[1] |= 1;
				}
				else if (mem_mask == 0x000000ff)
				{
					m_buserr[1] |= 3;
				}
			}
		}
		return 0;
	}
}

template void sun4_mmu_base_device::insn_data_w<sun4_mmu_base_device::USER_INSN>(const uint32_t, const uint32_t, const uint32_t);
template void sun4_mmu_base_device::insn_data_w<sun4_mmu_base_device::SUPER_INSN>(const uint32_t, const uint32_t, const uint32_t);
template void sun4_mmu_base_device::insn_data_w<sun4_mmu_base_device::USER_DATA>(const uint32_t, const uint32_t, const uint32_t);
template void sun4_mmu_base_device::insn_data_w<sun4_mmu_base_device::SUPER_DATA>(const uint32_t, const uint32_t, const uint32_t);

template <sun4_mmu_base_device::insn_data_mode MODE>
void sun4_mmu_base_device::insn_data_w(const uint32_t offset, const uint32_t data, const uint32_t mem_mask)
{
	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];// & m_pmeg_mask;
	const uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);

	if (m_page_valid[entry_index])
	{
		page_entry &entry = m_pagemap[entry_index];
		if ((!entry.writable) || (entry.supervisor && MODE != SUPER_DATA && MODE != SUPER_INSN))
		{
			LOGMASKED(LOG_WRITE_PROTECT, "write protect error with PTE %d (%08x), %08x = %08x & %08x, PC=%08x\n", entry_index, page_entry_to_uint(entry_index), offset << 2, data, mem_mask, m_cpu->pc());
			m_buserr[0] |= 0x8040;   // write, protection error
			m_buserr[1] = offset << 2;
			m_host->set_mae();
			return;
		}

		entry.accessed = PM_ACCESSED;
		entry.modified = PM_MODIFIED;

		const uint32_t tmp = entry.page | (offset & m_page_mask);

		switch (entry.type)
		{
		case 0: // type 0
			if (tmp < m_populated_ram_words)
			{
				const uint32_t set = (tmp >> 22) & 3;
				const uint32_t addr_mask = m_ram_set_mask[set];
				const uint32_t masked_addr = m_ram_set_base[set] + (tmp & addr_mask);
				COMBINE_DATA((m_ram_ptr + masked_addr));
			}
			else if (tmp >= 0x4000000 >> 2 && tmp < 0x10000000 >> 2)
			{
				type0_timeout_w(tmp);
			}
			return;

		case 1: // type 1
			m_type1_w(tmp, data, mem_mask);
			return;

		default:
			LOGMASKED(LOG_UNKNOWN_SPACE, "write unknown space type %d, %08x = %08x & %08x, PC=%08x\n", entry.type, tmp << 2, data, mem_mask, m_cpu->pc());
			m_host->set_mae();
			m_buserr[0] = 0x8020;
			m_buserr[1] = offset << 2;
			return;
		}
	}
	else
	{
		LOGMASKED(LOG_INVALID_PTE, "write invalid PTE %d (%08x), %08x = %08x & %08x, PC=%08x\n", entry_index, page_entry_to_uint(entry_index), offset << 2, data, mem_mask, m_cpu->pc());
		m_host->set_mae();
		m_buserr[0] |= 0x8080;    // write cycle, invalid PTE
		m_buserr[1] = offset << 2;
		if (mem_mask != ~0 && mem_mask != 0xffff0000 && mem_mask != 0xff000000)
		{
			if (mem_mask == 0x0000ffff || mem_mask == 0x0000ff00)
			{
				m_buserr[1] |= 2;
			}
			else if (mem_mask == 0x00ff0000)
			{
				m_buserr[1] |= 1;
			}
			else if (mem_mask == 0x000000ff)
			{
				m_buserr[1] |= 3;
			}
		}
	}
}

void sun4_mmu_base_device::l2p_command(int ref, const std::vector<std::string> &params)
{
	uint64_t addr, offset;

	if (!machine().debugger().commands().validate_number_parameter(params[0], addr)) return;

	addr &= 0xffffffff;
	offset = addr >> 2;

	uint8_t pmeg = 0;
	uint32_t entry_index = 0, tmp = 0;
	uint32_t entry_value = 0;

	pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
	entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	tmp = m_pagemap[entry_index].page | (offset & m_page_mask);
	entry_value = page_entry_to_uint(entry_index);

	if (m_page_valid[entry_index])
	{
		machine().debugger().console().printf("logical %08x => phys %08x, type %d (pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, m_pagemap[entry_index].type, pmeg, entry_index, entry_value);
	}
	else
	{
		machine().debugger().console().printf("logical %08x points to an invalid PTE! (pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, pmeg, entry_index, entry_value);
	}
}
