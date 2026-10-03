// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
//
// The original Mali-200 has no Mali-400 soft-reset command/completion IRQ.
// Geometry, fragment and raster execution are functional models. See the
// implementation files for currently unsupported formats and precision limits.

#include "emu.h"
#include "mali200.h"

#include <cmath>
#include <cstring>
#include <numbers>

#define LOG_REGS (1U << 1)
#define LOG_JOBS (1U << 2)
#define LOG_LISTS (1U << 3)
#define VERBOSE (0)
#include "logmacro.h"

#include "mali200_gp.hxx"
#include "mali200_plb.hxx"
#include "mali200_texture.hxx"
#include "mali200_pp.hxx"
#include "mali200_render.hxx"

DEFINE_DEVICE_TYPE(MALI200, mali200_device, "mali200", "ARM Mali-200 / MaliGP2")

mali200_device::mali200_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MALI200, tag, owner, clock)
		, m_dma(*this, finder_base::DUMMY_TAG, -1)
		, m_dma_page_permissions(*this, 0)
		, m_pp_irq(*this)
		, m_gp_irq(*this)
		, m_mmu_irq(*this)
{
}

void mali200_device::map(address_map &map)
{
	map(0x0000, 0x10ff).rw(FUNC(mali200_device::pp_r), FUNC(mali200_device::pp_w));
	map(0x2000, 0x20ff).rw(FUNC(mali200_device::gp_r), FUNC(mali200_device::gp_w));
	map(0x3000, 0x3023).rw(FUNC(mali200_device::mmu_r), FUNC(mali200_device::mmu_w));
}

void mali200_device::device_start()
{
	m_dma->cache(m_dma_access);
	m_pp_timer = timer_alloc(FUNC(mali200_device::pp_run), this);
	m_gp_timer = timer_alloc(FUNC(mali200_device::gp_run), this);
	save_item(NAME(m_pp));
	save_item(NAME(m_gp));
	save_item(NAME(m_mmu));
	save_item(NAME(m_gp_heap));
	save_item(NAME(m_gp_started));
	save_item(NAME(m_pp_jobs));
	save_item(NAME(m_gp_jobs));
	save_item(NAME(m_vs_config));
	save_item(NAME(m_vs_shader));
	save_item(NAME(m_vs_uniform));
	save_item(NAME(m_plb_config));
	save_item(NAME(m_plb_bins));
	save_item(NAME(m_plb_array));
	save_item(NAME(m_plb_stride));
	save_item(NAME(m_plb_state));
	save_item(NAME(m_plb_vertex));
	save_item(NAME(m_plb_scissor));
	save_item(NAME(m_plb_scissor_set));
	save_item(NAME(m_plb_append_done));
	machine().save().register_postload(save_prepost_delegate(FUNC(mali200_device::update_irqs), this));
}

void mali200_device::device_reset()
{
	m_pp_jobs = m_gp_jobs = 0;
	reset_pp();
	reset_gp();
	reset_mmu();
	update_irqs();
}

void mali200_device::reset_pp()
{
	m_pp_timer->adjust(attotime::never);
	std::fill(std::begin(m_pp), std::end(m_pp), 0);
	m_pp[0x0c / 4] = 2;
	m_pp[0x10 / 4] = 9;
	m_pp[0x48 / 4] = 0x75;
	// IGS38's r3p0 userspace library checks for the Mali-200 r0p5 core.
	m_pp[0x1000 / 4] = 0xc8070005;
	m_pp[0x1028 / 4] = 0x1ff;
	m_pp[0x1064 / 4] = 1000000;
}

void mali200_device::reset_gp()
{
	m_gp_timer->adjust(attotime::never);
	std::fill(std::begin(m_gp), std::end(m_gp), 0);
	m_gp[0x38 / 4] = 0xffffff00;
	m_gp[0x6c / 4] = 0x0a070100;
	m_gp[0xa4 / 4] = 1000000;
	m_gp_started = m_gp_heap[0] = m_gp_heap[1] = 0;
	reset_plb();
	std::fill(std::begin(m_vs_config), std::end(m_vs_config), 0);
	std::fill(std::begin(m_vs_shader), std::end(m_vs_shader), 0);
	std::fill(std::begin(m_vs_uniform), std::end(m_vs_uniform), 0);
	for (unsigned i = 0; i < 32; ++i)
		m_vs_config[i * 2 + 1] = 63;
	m_vs_config[0x42] = 0x0f000000;
}

void mali200_device::reset_mmu()
{
	std::fill(std::begin(m_mmu), std::end(m_mmu), 0);
	m_mmu[1] = 0x18;
}

void mali200_device::update_irqs()
{
	m_pp_irq(bool(m_pp[0x1020 / 4] & m_pp[0x1028 / 4]));
	m_gp_irq(bool(m_gp[0x24 / 4] & m_gp[0x2c / 4]));
	m_mmu_irq(bool(m_mmu[5] & m_mmu[7]));
}

u32 mali200_device::pp_r(offs_t offset)
{
	if (offset == 0x1008 / 4)
		return m_pp[offset] | ((m_pp[0x1020 / 4] & m_pp[0x1028 / 4]) ? 0x40 : 0);
	if (offset == 0x102c / 4)
		return m_pp[0x1020 / 4] & m_pp[0x1028 / 4];
	return m_pp[offset];
}

void mali200_device::pp_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const written = data & mem_mask;
	unsigned const reg = offset * 4;
	LOGMASKED(LOG_REGS, "PP write %04x=%08x mask=%08x\n", reg, data, mem_mask);
	switch (reg)
	{
	case 0x100c:
		pp_command(written);
		break;
	case 0x1020:
		m_pp[offset] |= written & 0x1ff;
		break;
	case 0x1024:
		m_pp[0x1020 / 4] &= ~written;
		break;
	case 0x1028:
		mem_mask &= 0x1ff;
		COMBINE_DATA(&m_pp[offset]);
		break;
	case 0x1000:
	case 0x1004:
	case 0x1008:
	case 0x102c:
	case 0x104c:
	case 0x1050:
		break;
	default:
		// Configuration registers; reserved holes are not writable storage.
		if ((reg <= 0x4c && reg != 0x38 && reg != 0x3c) || (reg >= 0x100 && reg <= 0x32c && (reg & 0xff) <= 0x2c) || reg == 0x1040 ||
				reg == 0x1044 || reg == 0x1048 || reg == 0x1060 || reg == 0x1064 || (reg >= 0x1080 && reg <= 0x108c) ||
				(reg >= 0x10a0 && reg <= 0x10ac))
		{
			if (reg == 0)
				mem_mask &= ~31U;
			if (reg == 4 || reg == 8)
				mem_mask &= ~63U;
			COMBINE_DATA(&m_pp[offset]);
		}
		break;
	}
	update_irqs();
}

void mali200_device::pp_command(u32 command)
{
	if (command & 0x20)
	{
		reset_pp();
		return;
	}
	if (command & 1)
	{
		m_pp[0x1008 / 4] |= 0x10;
		m_pp[0x1020 / 4] |= 0x20;
	}
	if (command & 2)
		m_pp[0x1008 / 4] &= ~0x10U;
	if (command & 0x10)
	{
		m_pp_timer->adjust(attotime::never);
		m_pp[0x1008 / 4] = (m_pp[0x1008 / 4] & ~1U) | 4;
		m_pp[0x1020 / 4] |= 8;
	}
	if ((command & 0x40) && !(m_pp[0x1008 / 4] & 0x1d))
	{
		++m_pp_jobs;
		m_pp[0x1008 / 4] |= 1;
		m_pp[0x1004 / 4] = m_pp[0];
		LOGMASKED(LOG_JOBS, "PP job %u list=%08x rsw=%08x vertex=%08x dte=%08x\n", m_pp_jobs, m_pp[0], m_pp[1], m_pp[2], m_mmu[0]);
		m_pp_timer->adjust(attotime::from_usec(10));
	}
}

u32 mali200_device::gp_r(offs_t offset)
{
	if (offset == 0x30 / 4)
		return m_gp[0x24 / 4] & m_gp[0x2c / 4];
	if (offset == 0x68 / 4)
		return m_gp[offset] | ((m_gp[0x24 / 4] & m_gp[0x2c / 4]) ? 1 : 0);
	return m_gp[offset];
}

void mali200_device::gp_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const written = data & mem_mask;
	unsigned const reg = offset * 4;
	LOGMASKED(LOG_REGS, "GP write %04x=%08x mask=%08x\n", reg, data, mem_mask);
	switch (reg)
	{
	case 0x20:
		gp_command(written);
		break;
	case 0x24:
		m_gp[offset] |= written & 0xbff;
		break;
	case 0x28:
		m_gp[0x24 / 4] &= ~written;
		break;
	case 0x2c:
		mem_mask &= 0xbff;
		COMBINE_DATA(&m_gp[offset]);
		break;
	case 0x10:
	case 0x14:
		mem_mask &= ~127U;
		COMBINE_DATA(&m_gp_heap[(reg - 0x10) / 4]);
		break;
	default:
		if (reg <= 0x0c || (reg >= 0x34 && reg <= 0x48) || reg == 0x54 || reg == 0x58 || reg == 0xa0 || reg == 0xa4)
		{
			if (reg <= 0x0c)
				mem_mask &= ~7U;
			COMBINE_DATA(&m_gp[offset]);
		}
		break;
	}
	update_irqs();
}

void mali200_device::gp_command(u32 command)
{
	if (command & 0x20)
	{
		reset_gp();
		return;
	}
	if (command & 0x200)
		m_gp[0x68 / 4] |= 4;
	if (command & 0x100)
		m_gp[0x68 / 4] &= ~4U;
	if (command & 0x10)
	{
		m_gp[0x10 / 4] = m_gp_heap[0];
		m_gp[0x14 / 4] = m_gp_heap[1];
		if (m_gp[0x68 / 4] & 0x20)
		{
			m_gp[0x68 / 4] &= ~0x20U;
			m_gp_timer->adjust(attotime::from_usec(10));
			LOGMASKED(LOG_JOBS, "PLB resume at %08x with heap %08x..%08x\n", m_gp[0x84 / 4], m_gp[4], m_gp[5]);
		}
	}
	if (command & 0x40)
	{
		m_gp_timer->adjust(attotime::never);
		m_gp[0x68 / 4] = (m_gp[0x68 / 4] & ~0xaU) | 0x80;
		m_gp[0x24 / 4] |= 0x40;
	}
	if ((command & 3) && !(m_gp[0x68 / 4] & 0xee))
	{
		++m_gp_jobs;
		m_gp_started = command & 3;
		m_gp[0x68 / 4] |= ((command & 1) ? 2 : 0) | ((command & 2) ? 8 : 0);
		m_gp[0x80 / 4] = m_gp[0];
		m_gp[0x84 / 4] = m_gp[2];
		if (command & 2)
			reset_plb();
		LOGMASKED(LOG_JOBS, "GP job %u units=%x vs=%08x..%08x plb=%08x..%08x heap=%08x..%08x dte=%08x\n", m_gp_jobs, m_gp_started, m_gp[0],
				m_gp[1], m_gp[2], m_gp[3], m_gp[4], m_gp[5], m_mmu[0]);
		m_gp_timer->adjust(attotime::from_usec(10));
	}
}

u32 mali200_device::mmu_r(offs_t offset)
{
	return (offset == 8) ? (m_mmu[5] & m_mmu[7]) : m_mmu[offset];
}

void mali200_device::mmu_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const written = data & mem_mask;
	LOGMASKED(LOG_REGS, "MMU write %04x=%08x mask=%08x\n", offset * 4, data, mem_mask);
	switch (offset)
	{
	case 0:
		mem_mask &= ~4095U;
		COMBINE_DATA(&m_mmu[0]);
		break;
	case 2:
		if (mem_mask != ~0U)
			break;
		switch (written)
		{
		case 0:
			m_mmu[1] |= 1;
			break;
		case 1:
			m_mmu[1] &= ~1U;
			break;
		case 2:
			m_mmu[1] |= 4;
			break;
		case 3:
			m_mmu[1] &= ~4U;
			break;
		case 4:
			break; // no translation cache
		case 5:
			m_mmu[1] &= ~2U;
			break;
		case 6:
			reset_mmu();
			break;
		}
		break;
	case 4:
		break; // no translation cache
	case 5:
		m_mmu[5] |= written & 3;
		break;
	case 6:
		m_mmu[5] &= ~written;
		break;
	case 7:
		mem_mask &= 3;
		COMBINE_DATA(&m_mmu[7]);
		break;
	}
	update_irqs();
}

void mali200_device::begin_dma_cache()
{
	m_dma_page_tags.fill(~0U);
	m_dma_table_pages.clear();
	m_dma_cache_active = true;
}

bool mali200_device::translate(u32 address, bool write, u32 &physical, unsigned bus)
{
	if (m_mmu[1] & 6)
		return false;
	physical = address;
	if (m_mmu[1] & 1)
	{
		unsigned const slot = (address >> 12) & (m_dma_page_tags.size() - 1);
		if (m_dma_cache_active && m_dma_page_tags[slot] == (address >> 12) && (m_dma_page_entries[slot] & (write ? 4 : 2)))
		{
			physical = (m_dma_page_entries[slot] & ~4095U) | (address & 4095);
			if (write && m_dma_table_pages.count(physical & ~4095U))
				m_dma_page_tags.fill(~0U);
			return bool(m_dma_page_access[slot] & (write ? 2 : 1));
		}
		u32 const dte_address = m_mmu[0] + ((address >> 22) * 4);
		if (!(m_dma_page_permissions(dte_address & ~4095U) & 1))
		{
			m_mmu[5] |= 2;
			update_irqs();
			return false;
		}
		if (m_dma_cache_active)
			m_dma_table_pages.insert(dte_address & ~4095U);
		u32 const dte = m_dma_access.read_dword(dte_address);
		u32 pte = 0;
		if (dte & 1)
		{
			u32 const pte_address = (dte & ~4095U) + (((address >> 12) & 1023) * 4);
			if (!(m_dma_page_permissions(pte_address & ~4095U) & 1))
			{
				m_mmu[5] |= 2;
				update_irqs();
				return false;
			}
			if (m_dma_cache_active)
				m_dma_table_pages.insert(pte_address & ~4095U);
			pte = m_dma_access.read_dword(pte_address);
		}
		if (!(dte & 1) || !(pte & 1) || !(pte & (write ? 4 : 2)))
		{
			m_mmu[1] = (m_mmu[1] & ~0x7e0U) | 2 | (write ? 0x20 : 0) | ((bus & 31) << 6);
			m_mmu[3] = address;
			m_mmu[5] |= 1;
			logerror("MMU page fault va=%08x write=%u bus=%u dte=%08x pte=%08x\n", address, write, bus, dte, pte);
			update_irqs();
			return false;
		}
		physical = (pte & ~4095U) | (address & 4095);
		u8 const access = m_dma_page_permissions(physical & ~4095U);
		if (m_dma_cache_active)
		{
			m_dma_page_tags[slot] = address >> 12;
			m_dma_page_entries[slot] = pte;
			m_dma_page_access[slot] = access;
			if (write && m_dma_table_pages.count(physical & ~4095U))
				m_dma_page_tags.fill(~0U);
		}
		return bool(access & (write ? 2 : 1));
	}
	return bool(m_dma_page_permissions(physical & ~4095U) & (write ? 2 : 1));
}

bool mali200_device::read_word(u32 address, u32 &data, unsigned bus)
{
	u32 physical;
	if ((address & 3) || !translate(address, false, physical, bus))
		return false;
	data = m_dma_access.read_dword(physical);
	return true;
}

bool mali200_device::write_word(u32 address, u32 data, unsigned bus)
{
	u32 physical;
	if ((address & 3) || !translate(address, true, physical, bus))
		return false;
	m_dma_access.write_dword(physical, data);
	return true;
}

void mali200_device::trace_list(char const *name, u32 start, u32 end, unsigned bus)
{
	for (u64 pos = start; pos < end && pos < u64(start) + 512; pos += 8)
	{
		u32 lo, hi;
		if (!read_word(pos, lo, bus) || !read_word(pos + 4, hi, bus))
			break;
		logerror("%s %08x: %08x %08x\n", name, u32(pos), lo, hi);
	}
}

TIMER_CALLBACK_MEMBER(mali200_device::pp_run)
{
	begin_dma_cache();
	m_pp_instructions.clear();
	m_pp_textures.clear();
	m_pp_instruction_cache.fill(nullptr);
	m_pp_texture_cache.fill(nullptr);
	bool const done = pp_render();
	m_dma_cache_active = false;
	m_pp[0x1008 / 4] &= ~1U;
	if (done)
		m_pp[0x1020 / 4] |= 1;
	else
	{
		m_pp[0x1008 / 4] |= 4;
		m_pp[0x1020 / 4] |= 4;
		logerror("PP rendering stopped at unsupported operation or memory fault\n");
	}
	update_irqs();
}

TIMER_CALLBACK_MEMBER(mali200_device::gp_run)
{
	begin_dma_cache();
	if ((VERBOSE & LOG_LISTS) && (m_gp_started & 1))
		trace_list("VS list", m_gp[0], m_gp[1], 5);
	if ((VERBOSE & LOG_LISTS) && (m_gp_started & 2))
		trace_list("PLB list", m_gp[2], m_gp[3], 7);
	bool const vertex_done = !(m_gp_started & 1) || vs_commands();
	if (vertex_done && (m_gp_started & 1))
	{
		m_gp_started &= ~1U;
		m_gp[0x68 / 4] &= ~2U;
		m_gp[0x24 / 4] |= 1;
	}
	bool const plb_done = !(m_gp_started & 2) || (vertex_done && plb_commands());
	if (vertex_done && plb_done)
	{
		m_gp[0x68 / 4] &= ~0xaU;
		m_gp[0x24 / 4] |= m_gp_started;
		m_gp_started = 0;
	}
	else if (!(m_gp[0x68 / 4] & 0x20))
	{
		m_gp[0x68 / 4] &= ~0xaU;
		m_gp[0x68 / 4] |= 0x80;
		m_gp[0x24 / 4] |= 0x20;
		logerror("GP incomplete command execution vs_done=%u plb_done=%u\n", vertex_done, plb_done);
	}
	m_dma_cache_active = false;
	update_irqs();
}
