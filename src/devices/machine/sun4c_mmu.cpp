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
#define LOG_TYPE1_TIMEOUT   (1U << 12)
#define LOG_UNKNOWN_SPACE   (1U << 13)
#define LOG_WRITE_PROTECT   (1U << 14)
#define LOG_READ_PROTECT    (1U << 15)
#define LOG_PARITY          (1U << 16)
#define LOG_ALL_ASI         (1U << 17) // WARNING: Heavy!
#define LOG_UNKNOWN_ASI     (1U << 18)
#define LOG_SEGMENT_FLUSH   (1U << 19)
#define LOG_PAGE_FLUSH      (1U << 20)
#define LOG_CONTEXT_FLUSH   (1U << 21)
#define LOG_CACHE_FILLS     (1U << 22)
#define LOG_PAGE_ENTRIES    (1U << 23)

#if SUN4CMMU_LOG_MEM_ACCESSES
static FILE* s_mem_log = nullptr;
#endif

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

void sun4_mmu_base_device::device_stop()
{
#if SUN4CMMU_LOG_MEM_ACCESSES
	fclose(s_mem_log);
#endif
}

void sun4_mmu_base_device::device_start()
{
#if SUN4CMMU_LOG_MEM_ACCESSES
	s_mem_log = fopen("sun4c_mem.bin", "wb");
	m_fpos = 0;
#endif

	m_type1_r.resolve_safe(0xffffffff);
	m_type1_w.resolve_safe();

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

	m_cache_word_size = m_cache_line_size >> 2;
	m_cache_tag_shift = 0;
	while ((m_cache_word_size & (1 << m_cache_tag_shift)) == 0)
	{
		m_cache_tag_shift++;
	}
	m_cache_vaddr_shift = 14;
	m_cache_tag_id_mask = 0xfffc;
	m_cache_tag_id_shift = 12;//13 + (m_cache_tag_shift - 2);
	m_cache_tag_mask = m_cache_mask >> m_cache_tag_shift;

	printf("m_cache_tag_shift %d\n", m_cache_tag_shift);
	printf("m_page_mask %08x\n", m_page_mask);
	printf("m_seg_entry_shift %08x\n", m_seg_entry_shift);
	printf("m_seg_entry_mask %08x\n", m_seg_entry_mask);
	printf("m_page_entry_mask %08x\n", m_page_entry_mask);
	printf("m_cache_mask %08x\n", m_cache_mask);
}

void sun4_mmu_base_device::device_reset()
{
	m_log_mem = false;
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

	memset(m_buserr, 0, sizeof(uint32_t) * 16);
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

uint32_t sun4_mmu_base_device::fetch_insn(const bool supervisor, uint32_t offset)
{
	if (supervisor)
		return insn_data_r<SUPER_INSN>(offset, 0xffffffff);
	else
		return insn_data_r<USER_INSN>(offset, 0xffffffff);
}

void sun4_mmu_base_device::hw_segment_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	for (uint32_t i = 0x0000; i < (0x1000 >> (m_cache_tag_shift + 2)); i++)
	{
		segment_flush_w(offset | (i << m_cache_tag_shift), data, mem_mask);
	}
}

void sun4_mmu_base_device::hw_page_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	for (uint32_t i = 0x0000; i < (0x1000 >> (m_cache_tag_shift + 2)); i++)
	{
		page_flush_w(offset | (i << m_cache_tag_shift), data, mem_mask);
	}
}

void sun4_mmu_base_device::hw_context_flush_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	for (uint32_t i = 0x0000; i < (0x1000 >> (m_cache_tag_shift + 2)); i++)
	{
		context_flush_w(offset | (i << m_cache_tag_shift), data, mem_mask);
	}
}

void sun4_mmu_base_device::hw_flush_all_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	for (uint32_t i = 0x0000; i < (0x1000 >> (m_cache_tag_shift + 2)); i++)
	{
		const uint32_t vaddr = offset | (i << m_cache_tag_shift);
		const uint32_t tag_addr = vaddr_to_cache_line(vaddr);
		m_cachetags[tag_addr] &= ~(1 << 19);
	}
}

uint32_t sun4_mmu_base_device::parity_r(uint32_t offset, uint32_t mem_mask)
{
	if (offset == 0)
	{
		const uint32_t data = m_parity_err_reg;
		LOGMASKED(LOG_PARITY, "%s: parity_err_reg read: %08x & %08x\n", machine().describe_context(), m_parity_err_reg, mem_mask);
		m_parity_err_reg &= ~0xcf;
		return data;
	}
	else if (offset == 1)
	{
		LOGMASKED(LOG_PARITY, "%s: memory_err_reg read: %08x & %08x\n", machine().describe_context(), m_memory_err_reg, mem_mask);
		return m_memory_err_reg;
	}
	return 0;
}

void sun4_mmu_base_device::parity_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		LOGMASKED(LOG_PARITY, "%s: parity_err_reg write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_parity_err_reg);
	}
	else
	{
		LOGMASKED(LOG_PARITY, "%s: memory_err_reg write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_memory_err_reg);
	}
}

void sun4_mmu_base_device::segment_flush_w(uint32_t vaddr, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SEGMENT_FLUSH, "%s: segment_flush_w %08x\n", machine().describe_context(), vaddr);
	const uint32_t tag_addr = vaddr_to_cache_line(vaddr);
	const uint32_t tag = m_cachetags[tag_addr];
	if ((tag & (1 << 19)) != 0 && ((tag >> 22) & m_ctx_mask) == m_context_masked)
	{
		// Unshifted
		// 00SS|SSSS|SSSS|SSPP|PPPP|BBBB|BBBB|BBBB
		// 00TT|TTTT|TTTT|TTTT|LLLL|LLLL|LLLL|bbbb
		//
		// 00ss|ssss|ssss|ss--|LLLL|LLLL|LLLL|----

		// Shifted
		// SSSS|SSSS|SSSS|PPPP|PPBB|BBBB|BBBB
		// TTTT|TTTT|TTTT|TTLL|LLLL|LLLL|LLww
		//
		// ssss|ssss|ssss|--LL|LLLL|LLLL|LL--

		const uint32_t tag_id = (tag >> 4) & m_cache_tag_mask;
		if (tag_id == ((vaddr >> 16) & 0xfff))
		{
			m_cachetags[tag_addr] &= ~(1 << 19);
			LOGMASKED(LOG_SEGMENT_FLUSH, "flushing line with tag %08x from vaddr %08x\n", tag, vaddr << 2);
		}
	}
}

void sun4_mmu_base_device::context_flush_w(uint32_t vaddr, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CONTEXT_FLUSH, "%s: context_flush_w %08x\n", machine().describe_context(), vaddr << 2);
	const uint32_t tag_addr = vaddr_to_cache_line(vaddr);
	const uint32_t tag = m_cachetags[tag_addr];
	if ((tag & (1 << 19)) != 0)
	{
		LOGMASKED(LOG_CONTEXT_FLUSH, "tag is valid: %08x (%d vs. %d)\n", tag, ((tag >> 22) & m_ctx_mask), m_context_masked);
		if (((tag >> 22) & m_ctx_mask) == m_context_masked && !BIT(tag, 20))
		{
			const uint32_t tag_id = (tag >> 2) & m_cache_tag_mask;
			if (tag_id == ((vaddr >> 14) & 0xfff))
			{
				LOGMASKED(LOG_CONTEXT_FLUSH, "flushing line with tag %08x from vaddr %08x\n", tag, vaddr << 2);
				m_cachetags[tag_addr] &= ~(1 << 19);
			}
		}
	}
}

void sun4_mmu_base_device::page_flush_w(uint32_t vaddr, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PAGE_FLUSH, "%s: page_flush_w %08x\n", machine().describe_context(), vaddr << 2);
	const uint32_t tag_addr = vaddr_to_cache_line(vaddr);
	const uint32_t tag = m_cachetags[tag_addr];
	if ((tag & (1 << 19)) != 0 && ((tag >> 22) & m_ctx_mask) == m_context_masked)
	{
		// Unshifted
		// 00SS|SSSS|SSSS|SSPP|PPPP|BBBB|BBBB|BBBB
		// 00TT|TTTT|TTTT|TTTT|LLLL|LLLL|LLLL|bbbb
		//
		// 00pp|pppp|pppp|pppp|pppp|LLLL|LLLL|---- ff1dc000
		// 1111|1111|0001|1101|1100|0000|0000|---- 0000fc70

		// Shifted
		// SSSS|SSSS|SSSS|PPPP|PPBB|BBBB|BBBB
		// TTTT|TTTT|TTTT|TTLL|LLLL|LLLL|LLww
		//
		// pppp|pppp|pppp|pppp|ppLL|LLLL|LL--

		const uint32_t tag_id = tag & m_cache_tag_id_mask;
		LOGMASKED(LOG_PAGE_FLUSH, "tag is valid: %08x (%04x vs. %04x)\n", tag, tag_id, (vaddr >> (m_cache_tag_id_shift + 2)) & 0x3fff);
		if (tag_id == ((vaddr >> m_cache_tag_id_shift) & 0xfffc))
		{
			m_cachetags[tag_addr] &= ~(1 << 19);
			LOGMASKED(LOG_PAGE_FLUSH, "flushing line with tag %08x from vaddr %08x\n", tag, vaddr << 2);
		}
	}
}

uint32_t sun4_mmu_base_device::context_reg_r(uint32_t offset, uint32_t mem_mask)
{
	if (mem_mask == 0x00ff0000)
	{
		LOGMASKED(LOG_CONTEXT, "sun4c_mmu: read context %08x & %08x = %08x\n", offset << 2, mem_mask, m_context<<16);
		return m_context<<16;
	}
	LOGMASKED(LOG_CONTEXT, "sun4c_mmu: read context %08x & %08x = %08x\n", offset << 2, mem_mask, m_context<<24);
	return m_context<<24;
}

uint32_t sun4_mmu_base_device::system_enable_r(uint32_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SYSTEM_ENABLE, "sun4c_mmu: read system enable %08x & %08x = %08x\n", offset << 2, mem_mask, m_system_enable<<24);
	return m_system_enable<<24;
}

uint32_t sun4_mmu_base_device::bus_error_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t ret = m_buserr[offset & 0xf];
	LOGMASKED(LOG_BUSERROR, "sun4c_mmu: read buserror %08x & %08x = %08x, PC=%x\n", 0x60000000 | (offset << 2), mem_mask, ret, m_cpu->pc());
	m_buserr[offset & 0xf] = 0; // clear on reading
	return ret;
}

uint32_t sun4_mmu_base_device::cache_tag_r(uint32_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_TAGS, "%s: sun4c_mmu: read dcache tags @ %x, %08x\n", machine().describe_context(), offset << 2, m_cachetags[vaddr_to_cache_line(offset)]);
	return m_cachetags[vaddr_to_cache_line(offset)];
}

uint32_t sun4_mmu_base_device::cache_data_r(uint32_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_DATA, "%s: sun4c_mmu: read dcache data @ %x, PC = %x\n", machine().describe_context(), offset << 2, m_cpu->pc());
	return m_cachedata[offset & 0x3fff];
}

uint32_t sun4_mmu_base_device::uart_r(uint32_t offset, uint32_t mem_mask)
{
	switch (offset & 3)
	{
		case 0: if (mem_mask == 0xff000000) return m_scc->cb_r(0)<<24; else return m_scc->db_r(0)<<8; break;
		case 1: if (mem_mask == 0xff000000) return m_scc->ca_r(0)<<24; else return m_scc->da_r(0)<<8; break;
	}
	return 0xffffffff;
}

void sun4_mmu_base_device::context_reg_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CONTEXT, "write context = %08x & %08x\n", data, mem_mask);
	m_context = data >> 24;
	m_context_masked = m_context & m_ctx_mask;
	m_cache_context = m_context & m_ctx_mask;
	m_curr_segmap = &m_segmap[m_context_masked][0];
	m_curr_segmap_masked = &m_segmap_masked[m_context_masked][0];
}

void sun4_mmu_base_device::system_enable_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
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
}

void sun4_mmu_base_device::bus_error_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t masked_offset = offset & 0xf;
	LOGMASKED(LOG_BUSERROR, "write bus error %08x = %08x & %08x\n", offset << 2, data, mem_mask);
	if (masked_offset == 0)
		m_buserr[0] = (data & 0x000000ff) | 0x00008000;
	else
		m_buserr[masked_offset] = data;
}

void sun4_mmu_base_device::cache_tag_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_TAGS, "%s: write dcache tags %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
	m_cachetags[vaddr_to_cache_line(offset)] = data & 0x03f8fffc;
}

void sun4_mmu_base_device::cache_data_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_DATA, "write cache data %08x = %08x & %08x\n", offset << 2, data, mem_mask);
	COMBINE_DATA(&m_cachedata[offset & 0x3fff]);
	m_cachetags[vaddr_to_cache_line(offset)] &= ~(1 << 19);
}

void sun4_mmu_base_device::uart_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset & 3)
	{
		case 0: if (mem_mask == 0xff000000) m_scc->cb_w(0, data>>24); else m_scc->db_w(0, data>>8); break;
		case 1: if (mem_mask == 0xff000000) m_scc->ca_w(0, data>>24); else { m_scc->da_w(0, data>>8); logerror("%c\n", data>>8); printf("%c", data>>8); } break;
	}
}

uint32_t sun4_mmu_base_device::segment_map_r(uint32_t offset, uint32_t mem_mask)
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

void sun4_mmu_base_device::segment_map_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
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

uint32_t sun4_mmu_base_device::page_map_r(uint32_t offset, uint32_t mem_mask)
{
	const uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	const uint32_t ret = page_entry_to_uint(page);
	LOGMASKED(LOG_PAGE_MAP, "read page map %08x & %08x (%x) = %08x\n", offset << 2, mem_mask, page, ret);
	return ret;
}

void sun4_mmu_base_device::page_map_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t page = m_curr_segmap_masked[(offset >> 16) & 0xfff] | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	LOGMASKED(LOG_PAGE_MAP, "write page map %08x (%x) = %08x & %08x\n", offset << 2, page, data, mem_mask);
	merge_page_entry(page, data, mem_mask);
	m_page_valid[page] = m_pagemap[page].valid;
}

void sun4_mmu_base_device::type0_timeout_r(uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "type 0 read timeout %08x, PC=%08x\n", offset << 2, m_cpu->pc());
	m_buserr[0] = 0x20; // read timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

void sun4_mmu_base_device::type0_timeout_w(uint32_t offset)
{
	LOGMASKED(LOG_TYPE0_TIMEOUT, "type 0 write timeout %08x, PC=%08x\n", offset << 2, m_cpu->pc());
	m_buserr[0] = 0x8020; // write timeout
	m_buserr[1] = 0x04000000 + (offset << 2);
	m_host->set_mae();
}

uint32_t sun4_mmu_base_device::type1_timeout_r(uint32_t offset)
{
	LOGMASKED(LOG_TYPE1_TIMEOUT, "type 1 read timeout %08x, PC=%08x\n", offset << 2, m_cpu->pc());
	m_buserr[2] = 0x20; // read timeout
	m_buserr[3] = m_type1_offset << 2;
	return 0;
}

void sun4_mmu_base_device::type1_timeout_w(uint32_t offset, uint32_t data)
{
	LOGMASKED(LOG_TYPE1_TIMEOUT, "type 1 write timeout %08x = %08x, PC=%08x\n", offset << 2, data, m_cpu->pc());
	m_buserr[2] = 0x120; // write timeout
	m_buserr[3] = m_type1_offset << 2;
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
	pe.raw = new_value;
	pe.index = index;
	LOGMASKED(LOG_PAGE_ENTRIES, "page entry %05x: data %08x, sanity %08x, mem_mask %08x, valid %d, write %d, super %d, cached %d\n", index, new_value, page_entry_to_uint(index), mem_mask, pe.valid ? 1 : 0, pe.writable ? 1 : 0, pe.supervisor ? 1 : 0, pe.uncached ? 0 : 1);
}

uint32_t sun4_mmu_base_device::vaddr_to_cache_line(uint32_t vaddr)
{
	return (vaddr >> m_cache_tag_shift) & m_cache_tag_mask;
}

void sun4_mmu_base_device::cache_fill(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t entry_index)
{
	const uint32_t cache_line = vaddr_to_cache_line(vaddr);
	m_cachetags[cache_line] = (1 << 19);
	m_cachetags[cache_line] |= entry.supervisor ? (1 << 20) : (0 << 20);
	m_cachetags[cache_line] |= entry.writable ? (1 << 21) : (0 << 21);
	m_cachetags[cache_line] |= (vaddr >> m_cache_tag_id_shift) & m_cache_tag_id_mask;
	m_cachetags[cache_line] |= (m_context_masked) << 22;
	const uint32_t cache_line_start = cache_line << m_cache_tag_shift;
	const uint32_t mem_line_mask = (1 << m_cache_tag_shift) - 1;
	const uint32_t mem_line_start = paddr & ~mem_line_mask;

	if (paddr < m_populated_ram_words)
	{
		LOGMASKED(LOG_CACHE_FILLS, "Filling cache line %04x (%08x) with data from paddr %08x, vaddr %08x, cache_line %04x, mem_line %08x, tag entry %04x\n",
			cache_line, 0x80000000 | (cache_line << m_cache_tag_shift), paddr << 2, vaddr << 2, cache_line_start << 2, mem_line_start << 2,
			(m_cachetags[cache_line] >> m_cache_line_size) & (0xfffc >> m_cache_tag_shift));
		LOGMASKED(LOG_CACHE_FILLS, "Tag: %08x, valid %d, super %d, write %d, ctx %d\n", m_cachetags[cache_line], BIT(m_cachetags[cache_line], 19),
			BIT(m_cachetags[cache_line], 20), BIT(m_cachetags[cache_line], 21), (m_cachetags[cache_line] >> 22) & 7);
		LOGMASKED(LOG_CACHE_FILLS, "Entry %05x: raw: %08x, valid %d, super %d, write %d, cache %d\n", entry_index, page_entry_to_uint(entry_index),
			entry.valid ? 1 : 0, entry.supervisor ? 1 : 0, entry.writable ? 1 : 0, entry.uncached ? 0 : 1);
		memcpy(&m_cachedata[cache_line_start], m_ram_ptr + mem_line_start, sizeof(uint32_t) * m_cache_word_size);
	}
	else
	{
		LOGMASKED(LOG_CACHE_FILLS, "Unable to fill cache line, paddr %08x exceeds populated RAM range\n", paddr << 2);
	}
}

template bool sun4_mmu_base_device::cache_fetch<sun4_mmu_base_device::USER_INSN>(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t &cached_data, uint32_t entry_index);
template bool sun4_mmu_base_device::cache_fetch<sun4_mmu_base_device::USER_DATA>(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t &cached_data, uint32_t entry_index);
template bool sun4_mmu_base_device::cache_fetch<sun4_mmu_base_device::SUPER_INSN>(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t &cached_data, uint32_t entry_index);
template bool sun4_mmu_base_device::cache_fetch<sun4_mmu_base_device::SUPER_DATA>(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t &cached_data, uint32_t entry_index);

template <sun4_mmu_base_device::insn_data_mode MODE>
bool sun4_mmu_base_device::cache_fetch(page_entry &entry, uint32_t vaddr, uint32_t paddr, uint32_t &cached_data, uint32_t entry_index)
{
	const uint32_t cache_line = vaddr_to_cache_line(vaddr);
	const uint32_t tag = m_cachetags[cache_line];
	if (!(tag & (1 << 19)))
	{
		// If the current tag is invalid, bail if the corresponding entry is invalid
		if (!entry.valid)
		{
			return false;
		}
		cache_fill(entry, vaddr, paddr, entry_index);
		cached_data = m_cachedata[vaddr & 0x3fff];
		return true;
	}
	else if ((tag & m_cache_tag_id_mask) == ((vaddr >> m_cache_tag_id_shift) & m_cache_tag_id_mask))
	{
		// If the current tag is valid and the tag IDs match, fetch from the cache
		if ((MODE >> 1) == USER_MODE)
		{
			// If we're in user mode and the context does not match, this is a miss
			if (((tag >> 22) & m_ctx_mask) != m_context_masked && (tag & (1 << 20)))
			{
				return false;
			}
		}
		cached_data = m_cachedata[vaddr & 0x3fff];
		return true;
	}
	else if (!entry.valid)
	{
		// If the current tag is valid, the tag IDs don't match, and the memory entry is invalid, miss
		return false;
	}
	else
	{
		// If the current tag is valid, the tag IDs don't match, and the memory entry is valid, it's a miss
		cache_fill(entry, vaddr, paddr, entry_index);
		cached_data = m_cachedata[vaddr & 0x3fff];
		return true;
	}
}

template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::USER_INSN>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::SUPER_INSN>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::USER_DATA>(const uint32_t, const uint32_t);
template uint32_t sun4_mmu_base_device::insn_data_r<sun4_mmu_base_device::SUPER_DATA>(const uint32_t, const uint32_t);

template <sun4_mmu_base_device::insn_data_mode MODE>
uint32_t sun4_mmu_base_device::insn_data_r(uint32_t offset, uint32_t mem_mask)
{
	// supervisor program fetches in boot state are special
	if (MODE == SUPER_INSN && m_fetch_bootrom)
	{
		if (!machine().side_effects_disabled())
			m_cpu->eat_cycles(50); // !?
		return m_rom_ptr[offset & 0x1ffff];
	}

	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];// & m_pmeg_mask;
	const uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	page_entry &entry = m_pagemap[entry_index];

	uint32_t cached_data = 0;
	if (entry.valid)
	{
		const uint32_t paddr = entry.page | (offset & m_page_mask);

		entry.accessed = PM_ACCESSED;

		{
			uint32_t tag_entry = vaddr_to_cache_line(offset);
			uint32_t tag = m_cachetags[tag_entry];
			bool user_mode = ((MODE >> 1) == USER_MODE);
			bool cache_hit = (tag & m_cache_tag_id_mask) == ((offset >> m_cache_tag_id_shift) & m_cache_tag_id_mask);
			bool cacheable = entry.type == 0 && !entry.uncached;
			bool check_cache = cacheable && cache_hit && (m_system_enable & ENA_CACHE);
			bool cache_valid = BIT(tag, 19);
			bool cache_protected = user_mode && BIT(tag, 20) && (((m_cachetags[tag_entry] >> 22) & m_ctx_mask) != m_context_masked);

			if (check_cache && cache_valid && cache_protected)
			{
				LOGMASKED(LOG_READ_PROTECT, "%s: read protect error with PTE %04x (%08x), %08x & %08x\n", machine().describe_context(),
					entry_index, page_entry_to_uint(entry_index), offset << 2, mem_mask);
				LOGMASKED(LOG_WRITE_PROTECT, "%s: tag %08x, line %04x, writable %d, check %d, valid %d, super %d, tag ctx %d, ctx reg %d\n",
					machine().describe_context(), m_cachetags[tag_entry], tag_entry, BIT(m_cachetags[tag_entry], 21), check_cache ? 1 : 0, cache_valid ? 1 : 0,
					BIT(m_cachetags[tag_entry], 20), (m_cachetags[tag_entry] >> 22) & 7, m_context_masked);
				LOGMASKED(LOG_WRITE_PROTECT, "%s: entry cached %d, entry writable %d, entry super %d\n",
					machine().describe_context(), entry.uncached ? 0 : 1, entry.writable ? 1 : 0, entry.supervisor ? 1 : 0);
				LOGMASKED(LOG_WRITE_PROTECT, "%s: pmeg %08x, seg entry %08x\n", machine().describe_context(), pmeg, ((offset >> m_seg_entry_shift) & m_seg_entry_mask));
				m_buserr[0] |= 0x0040;   // read protection error
				m_buserr[1] = offset << 2;
				m_host->set_mae();
				m_cachetags[tag_entry] &= ~(1 << 19);
				return 0;
			}
		}

		switch (entry.type)
		{
		case 0: // type 0 space
			if (paddr < m_populated_ram_words)
			{
				if (BIT(m_parity_err_reg, 4) && m_parity_err)
				{
					LOGMASKED(LOG_PARITY, "%s: ram read with parity %08x: %08x & %08x\n", machine().describe_context(), m_parity_err_reg, offset, mem_mask);
					m_parity_err_reg |= m_parity_err;
					if (BIT(m_parity_err_reg, 7))
					{
						m_parity_err_reg |= (1 << 6);
					}
					uint8_t boffs = 0;

					if (ACCESSING_BITS_24_31)
						boffs = 0;
					else if (ACCESSING_BITS_16_23)
						boffs = 1;
					else if (ACCESSING_BITS_8_15)
						boffs = 2;
					else if (ACCESSING_BITS_0_7)
						boffs = 3;

					m_parity_err_reg |= (1 << 7);
					m_parity_err = 0;
					m_buserr[0] = 0x8; // Read cycle, memory error
					m_buserr[1] = (offset << 2) | boffs;
					m_host->set_mae();
				}
				const uint32_t set = (paddr >> 22) & 3;
				const uint32_t addr_mask = m_ram_set_mask[set];
				const uint32_t masked_addr = m_ram_set_base[set] + (paddr & addr_mask);
				if (!entry.uncached && (m_system_enable & ENA_CACHE))
				{
					if (cache_fetch<MODE>(entry, offset, masked_addr, cached_data, entry_index))
					{
#if SUN4CMMU_LOG_MEM_ACCESSES
						uint32_t value = masked_addr | 0x80000000;
						fwrite(&value, 1, 4, s_mem_log);
						fwrite(&cached_data, 1, 4, s_mem_log);
						m_fpos += 8;
#endif
						return cached_data;
					}
				}
#if SUN4CMMU_LOG_MEM_ACCESSES
				uint32_t value = masked_addr | 0x80000000;
				fwrite(&value, 1, 4, s_mem_log);
				fwrite(&m_ram_ptr[masked_addr], 1, 4, s_mem_log);
				m_fpos += 8;
#endif
				return m_ram_ptr[masked_addr];
			}
			else if (paddr >= 0x4000000 >> 2 && paddr < 0x10000000 >> 2)
			{
				type0_timeout_r(paddr);
			}
			return ~0;

		case 1: // type 1 space
			m_type1_offset = offset;
			return m_type1_r(paddr, mem_mask);

		default:
			LOGMASKED(LOG_UNKNOWN_SPACE, "read unknown space type %d, %08x & %08x, PC=%08x\n", entry.type, paddr << 2, mem_mask, m_cpu->pc());
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
			if ((m_system_enable & ENA_CACHE) && cache_fetch<MODE>(entry, offset, 0, cached_data, entry_index))
			{
				return cached_data;
			}
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
void sun4_mmu_base_device::insn_data_w(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	// it's translation time
	const uint32_t pmeg = m_curr_segmap_masked[(offset >> 16) & 0xfff];// & m_pmeg_mask;
	const uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);

	if (m_page_valid[entry_index])
	{
		page_entry &entry = m_pagemap[entry_index];
		const uint32_t paddr = entry.page | (offset & m_page_mask);

		{
			uint32_t tag_entry = vaddr_to_cache_line(offset);
			uint32_t tag = m_cachetags[tag_entry];
			bool user_mode = (MODE >> 1) == USER_MODE;
			bool cacheable = entry.type == 0 && !entry.uncached;
			bool check_cache = cacheable && (m_system_enable & ENA_CACHE);
			bool cache_valid = BIT(tag, 19);
			bool cache_hit = (tag & m_cache_tag_id_mask) == ((offset >> m_cache_tag_id_shift) & m_cache_tag_id_mask);
			bool cache_writable = BIT(tag, 21);
			bool cache_protected = user_mode && BIT(tag, 20) && (((tag >> 22) & m_ctx_mask) != m_context_masked);

			if (cacheable && (!cache_valid || !cache_hit || !cache_writable || cache_protected))
			{
				m_cachetags[tag_entry] &= ~(1 << 19);
			}

			if ((check_cache && cache_hit && cache_valid && (!cache_writable || cache_protected)) ||
				(!check_cache && (!entry.writable || (user_mode && entry.supervisor))))
			{
				LOGMASKED(LOG_WRITE_PROTECT, "%s: write protect error with PTE %04x (%08x), %08x = %08x & %08x, mode %c\n", machine().describe_context(),
					entry_index, page_entry_to_uint(entry_index), offset << 2, data, mem_mask, user_mode ? 'U' : 'S');
				LOGMASKED(LOG_WRITE_PROTECT, "%s: tag %08x, line %04x, writable %d, check %d, valid %d, super %d, tag ctx %d, ctx reg %d\n",
					machine().describe_context(), m_cachetags[tag_entry], tag_entry, BIT(m_cachetags[tag_entry], 21), check_cache ? 1 : 0, cache_valid ? 1 : 0,
					BIT(m_cachetags[tag_entry], 20), (m_cachetags[tag_entry] >> 22) & 7, m_context_masked);
				LOGMASKED(LOG_WRITE_PROTECT, "%s: entry cached %d, entry writable %d, entry super %d\n",
					machine().describe_context(), entry.uncached ? 0 : 1, entry.writable ? 1 : 0, entry.supervisor ? 1 : 0);
				LOGMASKED(LOG_WRITE_PROTECT, "%s: pmeg %08x, seg entry %08x\n", machine().describe_context(), pmeg, ((offset >> m_seg_entry_shift) & m_seg_entry_mask));
				m_buserr[0] |= 0x8040;   // write protection error
				m_buserr[1] = offset << 2;
				m_host->set_mae();
				return;
			}
		}

		entry.accessed = PM_ACCESSED;
		entry.modified = PM_MODIFIED;

		switch (entry.type)
		{
		case 0: // type 0
			if (paddr < m_populated_ram_words)
			{
				if (BIT(m_parity_err_reg, 5))
				{
					LOGMASKED(LOG_PARITY, "%s: ram write with parity %08x: %08x = %08x & %08x\n", machine().describe_context(), m_parity_err_reg,
						offset, data, mem_mask);
					if (ACCESSING_BITS_24_31)
						m_parity_err |= (1 << 0);
					if (ACCESSING_BITS_16_23)
						m_parity_err |= (1 << 1);
					if (ACCESSING_BITS_8_15)
						m_parity_err |= (1 << 2);
					if (ACCESSING_BITS_0_7)
						m_parity_err |= (1 << 3);
				}
				const uint32_t set = (paddr >> 22) & 3;
				const uint32_t addr_mask = m_ram_set_mask[set];
				const uint32_t masked_addr = m_ram_set_base[set] + (paddr & addr_mask);
				if (!entry.uncached && (m_system_enable & ENA_CACHE))
				{
					const uint32_t cache_entry = vaddr_to_cache_line(offset);
					const uint32_t tag = m_cachetags[cache_entry];
					if (tag & (1 << 19))
					{
						if ((m_cachetags[cache_entry] & m_cache_tag_id_mask) == ((offset >> m_cache_tag_id_shift) & m_cache_tag_id_mask))
						{
							COMBINE_DATA(&m_cachedata[offset & 0x3fff]);
						}
						else
						{
							//m_cachetags[cache_entry] &= ~(1 << 19);
							//cache_fill(entry, offset, masked_addr, entry_index);
							//COMBINE_DATA(&m_cachedata[offset & 0x3fff]);
						}
					}
				}
				COMBINE_DATA((m_ram_ptr + masked_addr));
#if SUN4CMMU_LOG_MEM_ACCESSES
				fwrite(&masked_addr, 1, 4, s_mem_log);
				fwrite(&m_ram_ptr[masked_addr], 1, 4, s_mem_log);
				m_fpos += 8;
#endif
			}
			else if (paddr >= 0x4000000 >> 2 && paddr < 0x10000000 >> 2)
			{
				type0_timeout_w(paddr);
			}
			return;

		case 1: // type 1
			m_type1_offset = offset;
			m_type1_w(paddr, data, mem_mask);
			return;

		default:
			LOGMASKED(LOG_UNKNOWN_SPACE, "write unknown space type %d, %08x = %08x & %08x, PC=%08x\n", entry.type, paddr << 2, data, mem_mask, m_cpu->pc());
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

bool sun4_mmu_base_device::translate(uint32_t &addr)
{
	const uint32_t pmeg = m_curr_segmap_masked[(addr >> 16) & 0xfff];// & m_pmeg_mask;
	const uint32_t entry_index = pmeg | ((addr >> m_seg_entry_shift) & m_seg_entry_mask);
	const page_entry &entry = m_pagemap[entry_index];
	addr = entry.page | (addr & m_page_mask);
	return entry.valid;
}

void sun4_mmu_base_device::l2p_command(const std::vector<std::string> &params)
{
	uint64_t addr, offset;

	if (!machine().debugger().commands().validate_number_parameter(params[0], addr)) return;

	addr &= 0xffffffff;
	offset = addr >> 2;

	uint8_t pmeg = m_curr_segmap_masked[(addr >> 18) & 0xfff];
	uint32_t entry_index = pmeg | ((offset >> m_seg_entry_shift) & m_seg_entry_mask);
	uint32_t paddr = m_pagemap[entry_index].page | (offset & m_page_mask);
	uint32_t entry_value = page_entry_to_uint(entry_index);

	if (m_page_valid[entry_index])
	{
		machine().debugger().console().printf("logical %08x => phys %08x, type %d (pmeg %d, entry %d PTE %08x)\n", addr, paddr << 2, m_pagemap[entry_index].type, pmeg, entry_index, entry_value);
	}
	else
	{
		machine().debugger().console().printf("logical %08x points to an invalid PTE! (pmeg %d, entry %d PTE %08x)\n", addr, paddr << 2, pmeg, entry_index, entry_value);
	}
}
