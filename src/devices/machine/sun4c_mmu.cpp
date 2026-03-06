// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sun4c_mmu.cpp - Sun 4/4c MMU emulation

***************************************************************************/

#include "emu.h"
#include "sun4c_mmu.h"

#include "cpu/sparc/sparc.h"

#include "debug/debugcon.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(SUN4_MMU, sun4_mmu_device, "sun4_mmu", "Sun 4 MMU")
DEFINE_DEVICE_TYPE(SUN4C_MMU, sun4c_mmu_device, "sun4c_mmu", "Sun 4c MMU")

#define LOG_PAGE_MAP        (1U << 1)
#define LOG_SEGMENT_MAP     (1U << 2)
#define LOG_CONTEXT         (1U << 3)
#define LOG_SYSTEM_ENABLE   (1U << 4)
#define LOG_UART            (1U << 5)
#define LOG_PARITY          (1U << 6)
#define LOG_SEGMENT_FLUSH   (1U << 7)
#define LOG_PAGE_FLUSH      (1U << 8)
#define LOG_CONTEXT_FLUSH   (1U << 9)
#define LOG_ALL_FLUSH       (1U << 10)
#define LOG_CACHE_TAGS      (1U << 11)
#define LOG_CACHE_DATA      (1U << 12)
#define LOG_INVALID_PTE     (1U << 13)
#define LOG_BUSERROR        (1U << 14)
#define LOG_TYPE0_TIMEOUT   (1U << 15)
#define LOG_TYPE1_TIMEOUT   (1U << 16)
#define LOG_UNKNOWN_SPACE   (1U << 17)
#define LOG_UNKNOWN_SEGMENT (1U << 18)
#define LOG_WRITE_PROTECT   (1U << 19)
#define LOG_MMU             (LOG_PAGE_MAP | LOG_SEGMENT_MAP | LOG_CONTEXT)
#define LOG_MISC_HW         (LOG_SYSTEM_ENABLE | LOG_UART | LOG_PARITY)
#define LOG_FLUSHES         (LOG_SEGMENT_FLUSH | LOG_PAGE_FLUSH | LOG_CONTEXT_FLUSH | LOG_ALL_FLUSH)
#define LOG_CACHE           (LOG_CACHE_TAGS | LOG_CACHE_DATA | LOG_FLUSHES)
#define LOG_ERRORS          (LOG_INVALID_PTE | LOG_BUSERROR | LOG_TYPE0_TIMEOUT | LOG_TIME1_TIMEOUT | LOG_UNKNOWN_SPACE | LOG_UNKNOWN_SEGMENT | LOG_WRITE_PROTECT)

//#define VERBOSE (LOG_MMU | LOG_MISC_HW | LOG_FLUSHES | LOG_CACHE | LOG_ERRORS)
#include "logmacro.h"

#define PRINT_UART_DATA (0)

sun4_mmu_base_device::sun4_mmu_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_scc(*this, finder_base::DUMMY_TAG)
	, m_host(nullptr)
	, m_type1_r(*this, 0xffffffff)
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
	// allocate timer for system reset
	m_reset_timer = timer_alloc(FUNC(sun4_mmu_base_device::reset_off_tick), this);
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
	save_pointer(reinterpret_cast<uint8_t*>(m_pagemap.get()), "m_pagemap", sizeof(page_entry) * 16384);

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
	save_item(NAME(m_type1_offset));
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
	save_item(NAME(m_parity_err_reg));
	save_item(NAME(m_memory_err_reg));
	save_item(NAME(m_parity_err));

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("l2p", CMDFLAG_NONE, 1, 1, std::bind(&sun4_mmu_base_device::l2p_command, this, _1));
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
	m_type1_offset = 0;
	m_parity_err_reg = 0;
	m_memory_err_reg = 0;
	m_parity_err = 0;

	memset(m_buserr, 0, sizeof(uint32_t) * 4);
	for (int i = 0; i < 16; i++)
	{
		memset(&m_segmap[i][0], 0, 4096);
	}
	memset(&m_pagemap[0], 0, sizeof(page_entry) * 16384);
	memset(&m_cachetags[0], 0, sizeof(uint32_t) * 16384);
	memset(&m_cachedata[0], 0, sizeof(uint32_t) * 16384);
}

TIMER_CALLBACK_MEMBER(sun4_mmu_base_device::reset_off_tick)
{
	m_reset_timer->adjust(attotime::never);
	m_cpu->set_input_line(SPARC_RESET, CLEAR_LINE);
}

uint32_t sun4_mmu_base_device::fetch_insn(const bool supervisor, const uint32_t offset)
{
	if (supervisor)
		return insn_data_r<SUPER_INSN>(offset, 0xffffffff);
	else
		return insn_data_r<USER_INSN>(offset, 0xffffffff);
}

void sun4_mmu_base_device::segment_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_SEGMENT_FLUSH, "%s: segment_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::page_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_PAGE_FLUSH, "%s: page_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::context_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_CONTEXT_FLUSH, "%s: context_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::hw_segment_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_SEGMENT_FLUSH, "%s: segment_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::hw_page_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_PAGE_FLUSH, "%s: page_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::hw_context_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_CONTEXT_FLUSH, "%s: context_flush_w %08x & %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

void sun4_mmu_base_device::hw_flush_all_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// Do nothing for now
	LOGMASKED(LOG_ALL_FLUSH, "%s: hw_flush_all_w %08x = %08x: %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

uint32_t sun4_mmu_base_device::context_reg_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_context << (mem_mask == 0x00ff0000 ? 16 : 24);
	LOGMASKED(LOG_CONTEXT, "%s: context_reg_r %08x & %08x: %08x\n", machine().describe_context(), offset << 2, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::context_reg_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CONTEXT, "%s: context_reg_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
	m_context = data >> 24;
	m_context_masked = m_context & m_ctx_mask;
	m_cache_context = m_context & m_ctx_mask;
	m_curr_segmap = &m_segmap[m_context_masked][0];
	m_curr_segmap_masked = &m_segmap_masked[m_context_masked][0];
}

uint32_t sun4_mmu_base_device::system_enable_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_system_enable << 24;
	LOGMASKED(LOG_SYSTEM_ENABLE, "%s: system_enable_r %08x & %08x: %08x\n", machine().describe_context(), offset << 2, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::system_enable_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SYSTEM_ENABLE, "%s: system_enable_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
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
}

uint32_t sun4_mmu_base_device::bus_error_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_buserr[offset & 0xf];
	LOGMASKED(LOG_BUSERROR, "%s: bus_error_r %08x & %08x: %08x\n", machine().describe_context(), 0x60000000 | (offset << 2), mem_mask, data);
	m_buserr[offset & 0xf] = 0; // clear on reading
	return data;
}

void sun4_mmu_base_device::bus_error_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t masked_offset = offset & 0xf;
	LOGMASKED(LOG_BUSERROR, "%s: bus_error_w: %08x = %08x & %08x\n", machine().describe_context(), 0x60000000 | (offset << 2), data, mem_mask);
	if (masked_offset == 0)
		m_buserr[0] = (data & 0x000000ff) | 0x00008000;
	else if (masked_offset == 1)
		m_buserr[1] = data;
	else if (masked_offset == 2)
		m_buserr[2] = data & 0x000000b0;
	else if (masked_offset == 3)
		m_buserr[3] = (data & 0x3fffffff) | ((data & 0x20000000) << 1) | ((data & 0x20000000) << 2);
}

uint32_t sun4_mmu_base_device::cache_tag_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_cachetags[offset & m_cache_mask];
	LOGMASKED(LOG_CACHE_TAGS, "%s: cache_tag_r %08x & %08x: %08x\n", machine().describe_context(), offset, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::cache_tag_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_TAGS, "%s: cache_tag_w: %08x = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_cachetags[offset & m_cache_mask] = data & 0x03f8fffc;
}

uint32_t sun4_mmu_base_device::cache_data_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_cachedata[offset & m_cache_mask];
	LOGMASKED(LOG_CACHE_DATA, "%s: cache_data_r %08x & %08x: %08x\n", machine().describe_context(), offset, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::cache_data_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_DATA, "%s: cache_data_w: %08x = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_cachedata[offset & m_cache_mask] = data | (1 << 19);
}

uint32_t sun4_mmu_base_device::uart_r(uint32_t offset, uint32_t mem_mask)
{
	uint32_t data = 0xffffffff;
	switch (offset & 3)
	{
		case 0:
			if (mem_mask == 0xff000000)
				data = m_scc->cb_r(0) << 24;
			else
				data = m_scc->db_r(0) << 8;
			break;
		case 1:
			if (mem_mask == 0xff000000)
				data = m_scc->ca_r(0) << 24;
			else
				data = m_scc->da_r(0) << 8;
			break;
	}
	LOGMASKED(LOG_UART, "%s: uart_r %08x & %08x: %08x\n", machine().describe_context(), offset, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::uart_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_UART, "%s: uart_w: %08x = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	switch (offset & 3)
	{
		case 0:
			if (mem_mask == 0xff000000)
				m_scc->cb_w(0, data >> 24);
			else
				m_scc->db_w(0, data >> 8);
			return;
		case 1:
			if (mem_mask == 0xff000000)
				m_scc->ca_w(0, data >> 24);
			else
			{
				m_scc->da_w(0, data >> 8);
				if (PRINT_UART_DATA)
				{
					printf("%c", data >> 8);
				}
			}
			return;
	}
}

uint32_t sun4_mmu_base_device::segment_map_r(uint32_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	if (mem_mask == 0xffff0000)
		data = m_curr_segmap[(offset >> 16) & 0xfff] << 16;
	else if (mem_mask == 0xff000000)
		data = m_curr_segmap[(offset >> 16) & 0xfff] << 24;
	else if (mem_mask == 0xffffffff)
		data = m_curr_segmap[(offset >> 16) & 0xfff];
	else
		LOGMASKED(LOG_UNKNOWN_SEGMENT, "%s: segment_map_r: %08x & %08x (unknown mask)\n", machine().describe_context(), offset << 2, mem_mask);
	LOGMASKED(LOG_SEGMENT_MAP, "%s: segment_map_r: %08x & %08x: %08x\n", machine().describe_context(), offset << 2, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::segment_map_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SEGMENT_MAP, "%s: segment_map_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);

	uint8_t segdata = 0;
	if (mem_mask == 0xffff0000)
		segdata = (data >> 16) & 0xff;
	else if (mem_mask == 0xff000000)
		segdata = (data >> 24) & 0xff;
	else if (mem_mask == 0xffffffff)
		segdata = data & 0xff;
	else
		LOGMASKED(LOG_UNKNOWN_SEGMENT, "%s: segment_map_w: %08x = %08x & %08x (unknown mask)\n", machine().describe_context(), offset << 2, data, mem_mask);

	const uint32_t seg = (offset >> 16) & 0xfff;
	m_curr_segmap[seg] = segdata;
	m_curr_segmap_masked[seg] = (segdata & m_pmeg_mask) << 6;
}

uint32_t sun4_mmu_base_device::page_map_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	const uint32_t data = page_entry_to_uint(page);
	LOGMASKED(LOG_PAGE_MAP, "%s: page_map_r: %08x (%x) & %08x: %08x\n", machine().describe_context(), offset << 2, page, mem_mask, data);
	return data;
}

void sun4_mmu_base_device::page_map_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	LOGMASKED(LOG_PAGE_MAP, "%s: page_map_w: %08x (%x) = %08x & %08x\n", machine().describe_context(), offset << 2, page, data, mem_mask);
	merge_page_entry(page, data, mem_mask);
	m_page_valid[page] = m_pagemap[page].valid;
}

void sun4_mmu_base_device::type0_timeout_r(const uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "%s: type0_timeout_r (%08x)\n", machine().describe_context(), offset << 2);
	m_buserr[0] = 0x20; // read timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

void sun4_mmu_base_device::type0_timeout_w(const uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "%s: type0_timeout_w (%08x)\n", machine().describe_context(), offset << 2);
	m_buserr[0] = 0x8020; // write timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

uint32_t sun4_mmu_base_device::type1_timeout_r(uint32_t offset)
{
	LOGMASKED(LOG_TYPE1_TIMEOUT, "%s: type1_timeout_r (%08x)\n", machine().describe_context(), offset << 2);
	m_buserr[2] = 0x20; // read timeout
	m_buserr[3] = m_type1_offset << 2;
	return 0;
}

void sun4_mmu_base_device::type1_timeout_w(uint32_t offset, uint32_t data)
{
	LOGMASKED(LOG_TYPE1_TIMEOUT, "%s: type1_timeout_w (%08x)\n", machine().describe_context(), offset << 2);
	m_buserr[2] = 0x120; // write timeout
	m_buserr[3] = m_type1_offset << 2;
}

uint32_t sun4_mmu_base_device::parity_r(uint32_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	if (offset == 0)
	{
		data = m_parity_err_reg;
		LOGMASKED(LOG_PARITY, "%s: parity_r (parity error register): %08x & %08x: %08x\n", machine().describe_context(), offset << 2, mem_mask, data);
		m_parity_err_reg &= ~0xcf;
	}
	else if (offset == 1)
	{
		data = m_memory_err_reg;
		LOGMASKED(LOG_PARITY, "%s: parity_r (memory error register): %08x & %08x: %08x\n", machine().describe_context(), offset << 2, mem_mask, data);
	}
	return data;
}

void sun4_mmu_base_device::parity_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		LOGMASKED(LOG_PARITY, "%s: parity_w (parity error register): %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		COMBINE_DATA(&m_parity_err_reg);
	}
	else
	{
		LOGMASKED(LOG_PARITY, "%s: parity_w (memory error register): %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		COMBINE_DATA(&m_memory_err_reg);
	}
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
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
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
			m_type1_offset = offset;
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
			m_type1_offset = offset;
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

void sun4_mmu_base_device::l2p_command(const std::vector<std::string_view> &params)
{
	uint64_t addr;
	if (!machine().debugger().console().validate_number_parameter(params[0], addr)) return;

	addr &= 0xffffffff;
	uint64_t offset = addr >> 2;

	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];
	const uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	const uint32_t tmp = m_pagemap[entry_index].page | (offset & m_page_mask);
	const uint32_t entry_value = page_entry_to_uint(entry_index);

	if (m_page_valid[entry_index])
	{
		machine().debugger().console().printf("logical %08x => phys %08x, type %d (pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, m_pagemap[entry_index].type, pmeg, entry_index, entry_value);
	}
	else
	{
		machine().debugger().console().printf("logical %08x points to an invalid PTE! (tmp %08x, pmeg %d, entry %d PTE %08x)\n", addr, tmp << 2, pmeg, entry_index, entry_value);
	}
}

