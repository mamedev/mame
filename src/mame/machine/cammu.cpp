// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Fairchild/Intergraph Cache and Memory Management
 * Unit (CAMMU) designed for use with the CLIPPER CPU family.
 *
 * The C100 and C300 designs used a pair of identical CAMMU devices, each
 * containing a cache, TLB and dynamic translation unit. One device was
 * configured and used for instruction memory, the other for data. It is
 * possible to write to multiple CAMMU devices sharing a common system bus by
 * using "global" register addresses.
 *
 * C400 designs initially implemented the memory management and cache functions
 * using discrete logic, later using a more highly integrated memory management
 * implementation, but still using discrete cache memory. In these systems, the
 * mmu is consolidated into a single logical unit handling both instruction and
 * data memory, with distinctly different program-visible architectures on the
 * C4I and C4E/T devices. Almost no documentation for these has been located.
 *
 * Primary reference: http://bitsavers.org/pdf/fairchild/clipper/CLIPPER%20C300%2032-Bit%20Compute%20Engine.pdf
 * Another reference: http://www.eecs.berkeley.edu/Pubs/TechRpts/1986/CSD-86-289.pdf
 *
 * TODO
 *   - c4 variants
 *   - fault register values
 *   - cache
 *   - bus errors
 */

#include "emu.h"
#include "cammu.h"

#include <algorithm>

#define LOG_GENERAL (1U << 0)
#define LOG_ACCESS  (1U << 1)
#define LOG_DTU     (1U << 2)
#define LOG_TLB     (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_ACCESS | LOG_DTU)
#include "logmacro.h"

// each variant of the cammu has different registers and a different addressing map
void cammu_c4t_device::map(address_map &map)
{
	map(0x008, 0x00b).rw(FUNC(cammu_c4t_device::ram_line_r), FUNC(cammu_c4t_device::ram_line_w));
	map(0x010, 0x013).rw(FUNC(cammu_c4t_device::s_pdo_r), FUNC(cammu_c4t_device::s_pdo_w));
	map(0x018, 0x01b).rw(FUNC(cammu_c4t_device::u_pdo_r), FUNC(cammu_c4t_device::u_pdo_w));
	map(0x020, 0x023).rw(FUNC(cammu_c4t_device::htlb_offset_r), FUNC(cammu_c4t_device::htlb_offset_w));
	map(0x028, 0x02b).rw(FUNC(cammu_c4t_device::i_fault_r), FUNC(cammu_c4t_device::i_fault_w));
	map(0x030, 0x033).rw(FUNC(cammu_c4t_device::fault_address_1_r), FUNC(cammu_c4t_device::fault_address_1_w));
	map(0x038, 0x03b).rw(FUNC(cammu_c4t_device::fault_address_2_r), FUNC(cammu_c4t_device::fault_address_2_w));
	map(0x040, 0x043).rw(FUNC(cammu_c4t_device::fault_data_1_lo_r), FUNC(cammu_c4t_device::fault_data_1_lo_w));
	map(0x048, 0x04b).rw(FUNC(cammu_c4t_device::fault_data_1_hi_r), FUNC(cammu_c4t_device::fault_data_1_hi_w));
	map(0x050, 0x053).rw(FUNC(cammu_c4t_device::fault_data_2_lo_r), FUNC(cammu_c4t_device::fault_data_2_lo_w));
	map(0x058, 0x05b).rw(FUNC(cammu_c4t_device::fault_data_2_hi_r), FUNC(cammu_c4t_device::fault_data_2_hi_w));
	map(0x060, 0x063).rw(FUNC(cammu_c4t_device::c4_bus_poll_r), FUNC(cammu_c4t_device::c4_bus_poll_w));
	map(0x068, 0x06b).rw(FUNC(cammu_c4t_device::control_r), FUNC(cammu_c4t_device::control_w));
	map(0x070, 0x073).rw(FUNC(cammu_c4t_device::bio_control_r), FUNC(cammu_c4t_device::bio_control_w));
	map(0x078, 0x07b).rw(FUNC(cammu_c4t_device::bio_address_tag_r), FUNC(cammu_c4t_device::bio_address_tag_w));

	map(0x100, 0x103).rw(FUNC(cammu_c4t_device::cache_data_lo_r), FUNC(cammu_c4t_device::cache_data_lo_w));
	map(0x104, 0x107).rw(FUNC(cammu_c4t_device::cache_data_hi_r), FUNC(cammu_c4t_device::cache_data_hi_w));
	map(0x108, 0x10b).rw(FUNC(cammu_c4t_device::cache_cpu_tag_r), FUNC(cammu_c4t_device::cache_cpu_tag_w));
	map(0x10c, 0x10f).rw(FUNC(cammu_c4t_device::cache_system_tag_valid_r), FUNC(cammu_c4t_device::cache_system_tag_valid_w));
	map(0x110, 0x113).rw(FUNC(cammu_c4t_device::cache_system_tag_r), FUNC(cammu_c4t_device::cache_system_tag_w));
	map(0x118, 0x11b).rw(FUNC(cammu_c4t_device::tlb_va_line_r), FUNC(cammu_c4t_device::tlb_va_line_w));
	map(0x11c, 0x11f).rw(FUNC(cammu_c4t_device::tlb_ra_line_r), FUNC(cammu_c4t_device::tlb_ra_line_w));
}

void cammu_c4i_device::map(address_map &map)
{
	map(0x000, 0x003).rw(FUNC(cammu_c4i_device::reset_r), FUNC(cammu_c4i_device::reset_w));
	map(0x010, 0x013).rw(FUNC(cammu_c4i_device::s_pdo_r), FUNC(cammu_c4i_device::s_pdo_w));
	map(0x018, 0x01b).rw(FUNC(cammu_c4i_device::u_pdo_r), FUNC(cammu_c4i_device::u_pdo_w));
	map(0x020, 0x023).rw(FUNC(cammu_c4i_device::clr_s_data_tlb_r), FUNC(cammu_c4i_device::clr_s_data_tlb_w));
	map(0x028, 0x02b).rw(FUNC(cammu_c4i_device::clr_u_data_tlb_r), FUNC(cammu_c4i_device::clr_u_data_tlb_w));
	map(0x030, 0x033).rw(FUNC(cammu_c4i_device::clr_s_insn_tlb_r), FUNC(cammu_c4i_device::clr_s_insn_tlb_w));
	map(0x038, 0x03b).rw(FUNC(cammu_c4i_device::clr_u_insn_tlb_r), FUNC(cammu_c4i_device::clr_u_insn_tlb_w));

	map(0x068, 0x06b).rw(FUNC(cammu_c4i_device::control_r), FUNC(cammu_c4i_device::control_w));

	map(0x080, 0x083).rw(FUNC(cammu_c4i_device::test_data_r), FUNC(cammu_c4i_device::test_data_w));
	map(0x088, 0x08b).rw(FUNC(cammu_c4i_device::i_fault_r), FUNC(cammu_c4i_device::i_fault_w));
	map(0x090, 0x093).rw(FUNC(cammu_c4i_device::fault_address_1_r), FUNC(cammu_c4i_device::fault_address_1_w));
	map(0x098, 0x09b).rw(FUNC(cammu_c4i_device::fault_address_2_r), FUNC(cammu_c4i_device::fault_address_2_w));
	map(0x0a0, 0x0a3).rw(FUNC(cammu_c4i_device::fault_data_1_lo_r), FUNC(cammu_c4i_device::fault_data_1_lo_w));
	map(0x0a8, 0x0ab).rw(FUNC(cammu_c4i_device::fault_data_1_hi_r), FUNC(cammu_c4i_device::fault_data_1_hi_w));
	map(0x0b0, 0x0b3).rw(FUNC(cammu_c4i_device::fault_data_2_lo_r), FUNC(cammu_c4i_device::fault_data_2_lo_w));
	map(0x0b8, 0x0bb).rw(FUNC(cammu_c4i_device::fault_data_2_hi_r), FUNC(cammu_c4i_device::fault_data_2_hi_w));
	map(0x0c0, 0x0c3).rw(FUNC(cammu_c4i_device::test_address_r), FUNC(cammu_c4i_device::test_address_w));
}

DEFINE_DEVICE_TYPE(CAMMU_C4T, cammu_c4t_device, "c4t", "C4E/C4T CAMMU")
DEFINE_DEVICE_TYPE(CAMMU_C4I, cammu_c4i_device, "c4i", "C4I CAMMU")
DEFINE_DEVICE_TYPE(CAMMU_C3,  cammu_c3_device,  "c3",  "C1/C3 CAMMU")

cammu_c4t_device::cammu_c4t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_c4_device(mconfig, CAMMU_C4T, tag, owner, clock)
	, m_ram_line(0)
	, m_htlb_offset(0)
	, m_c4_bus_poll(0)
	, m_bio_control(0)
	, m_bio_address_tag(0)
	, m_cache_data_lo(0)
	, m_cache_data_hi(0)
	, m_cache_cpu_tag(0)
	, m_cache_system_tag_valid(0)
	, m_cache_system_tag(0)
	, m_tlb_va_line(0)
	, m_tlb_ra_line(0)
{
}

cammu_c4i_device::cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_c4_device(mconfig, CAMMU_C4I, tag, owner, clock)
	, m_reset(0)
	, m_clr_s_data_tlb(0)
	, m_clr_u_data_tlb(0)
	, m_clr_s_insn_tlb(0)
	, m_clr_u_insn_tlb(0)
	, m_test_data(0)
	, m_test_address(0)
{
}

cammu_c4_device::cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, type, tag, owner, clock)
	, m_s_pdo(0)
	, m_u_pdo(0)
	, m_control(0)
	, m_i_fault(0)
	, m_fault_address_1(0)
	, m_fault_address_2(0)
	, m_fault_data_1_lo(0)
	, m_fault_data_1_hi(0)
	, m_fault_data_2_lo(0)
	, m_fault_data_2_hi(0)
{
}

cammu_c3_device::cammu_c3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, CAMMU_C3, tag, owner, clock)
	, m_linked{ this }
	, m_s_pdo(0)
	, m_u_pdo(0)
	, m_fault(0)
	, m_control(CID_C3)
{
}

cammu_device::cammu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_exception_func(*this)
{
}

void cammu_device::device_start()
{
	m_exception_func.resolve();
}

void cammu_device::device_reset()
{
}

void cammu_c4_device::device_start()
{
	cammu_device::device_start();

	save_item(NAME(m_s_pdo));
	save_item(NAME(m_u_pdo));
	save_item(NAME(m_control));

	save_item(NAME(m_i_fault));
	save_item(NAME(m_fault_address_1));
	save_item(NAME(m_fault_address_2));
	save_item(NAME(m_fault_data_1_lo));
	save_item(NAME(m_fault_data_1_hi));
	save_item(NAME(m_fault_data_2_lo));
	save_item(NAME(m_fault_data_2_hi));
}

void cammu_c4i_device::device_start()
{
	cammu_c4_device::device_start();

	save_item(NAME(m_reset));
	save_item(NAME(m_clr_s_data_tlb));
	save_item(NAME(m_clr_u_data_tlb));
	save_item(NAME(m_clr_s_insn_tlb));
	save_item(NAME(m_clr_u_insn_tlb));
	save_item(NAME(m_test_data));
	save_item(NAME(m_test_address));
}

void cammu_c4t_device::device_start()
{
	cammu_c4_device::device_start();

	save_item(NAME(m_ram_line));
	save_item(NAME(m_htlb_offset));
	save_item(NAME(m_c4_bus_poll));
	save_item(NAME(m_bio_control));
	save_item(NAME(m_bio_address_tag));

	save_item(NAME(m_cache_data_lo));
	save_item(NAME(m_cache_data_hi));
	save_item(NAME(m_cache_cpu_tag));
	save_item(NAME(m_cache_system_tag_valid));
	save_item(NAME(m_cache_system_tag));
	save_item(NAME(m_tlb_va_line));
	save_item(NAME(m_tlb_ra_line));
}

void cammu_c3_device::device_start()
{
	cammu_device::device_start();

	save_item(NAME(m_s_pdo));
	save_item(NAME(m_u_pdo));
	save_item(NAME(m_fault));
	save_item(NAME(m_control));

	for (tlb_set_t &tlb_set : m_tlb)
	{
		tlb_set.u = false;

		tlb_set.w.ra = tlb_set.w.va = 0;
		m_memory[ST0].space->cache(tlb_set.w.cache);

		tlb_set.x.ra = tlb_set.x.va = 0;
		m_memory[ST0].space->cache(tlb_set.x.cache);
	}
}

void cammu_c3_device::device_reset()
{
	cammu_device::device_reset();

	m_control = (m_control & CNTL_CID) | (CNTL_ATE | UST_3 | CNTL_EWIR | CNTL_EWIW | CNTL_EWCW | CNTL_EP);
}

void cammu_device::set_spaces(address_space &main_space, address_space &io_space, address_space &boot_space)
{
	m_memory[ST0].space = &main_space;
	m_memory[ST1].space = &main_space;
	m_memory[ST2].space = &main_space;
	m_memory[ST3].space = &main_space;

	m_memory[ST4].space = &io_space;
	m_memory[ST5].space = &boot_space;
	m_memory[ST6].space = &main_space;

	// FIXME: this tag is probably not used, but if it is, need to figure
	// out how to implement it properly.
	m_memory[ST7].space = &main_space;

	for (memory_t &memory : m_memory)
		memory.space->cache(memory.cache);
}

bool cammu_device::memory_translate(const u32 ssw, const int spacenum, const int intention, offs_t &address)
{
	// translate the address
	translated_t translated = translate_address(ssw, address, BYTE,
		(intention & TRANSLATE_TYPE_MASK) == TRANSLATE_READ ? READ :
		(intention & TRANSLATE_TYPE_MASK) == TRANSLATE_WRITE ? WRITE :
		EXECUTE);

	// check that the requested space number matches the mapped space
	if (translated.cache && translated.cache->space().spacenum() == spacenum)
	{
		address = translated.address;

		return true;
	}

	return false;
}

cammu_device::translated_t cammu_device::translate_address(const u32 ssw, const u32 virtual_address, const access_size size, const access_type mode)
{
	// get effective user/supervisor mode
	const bool user = (mode == EXECUTE) ? (ssw & SSW_U) : (ssw & (SSW_U | SSW_UU));

	// check for alignment faults
	if (!machine().side_effects_disabled() && get_alignment())
	{
		if ((mode == EXECUTE && (virtual_address & 0x1)) || (mode != EXECUTE && virtual_address & (size - 1)))
		{
			set_fault(virtual_address, mode == EXECUTE ? EXCEPTION_I_ALIGNMENT_FAULT : EXCEPTION_D_ALIGNMENT_FAULT);

			return { nullptr, 0 };
		}
	}

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if (!user && (virtual_address & ~0x7fff) == 0)
	{
		switch (virtual_address & 0x7000)
		{
			// pages 0-3: main space pages 0-3
		case 0x0000: return { &m_memory[ST1].cache, virtual_address & 0x3fff };
		case 0x1000: return { &m_memory[ST2].cache, virtual_address & 0x3fff };
		case 0x2000: return { &m_memory[ST3].cache, virtual_address & 0x3fff };
		case 0x3000: return { &m_memory[ST3].cache, virtual_address & 0x3fff };

			// pages 4-5: i/o space pages 0-1
		case 0x4000: return { &m_memory[ST4].cache, virtual_address & 0x1fff };
		case 0x5000: return { &m_memory[ST4].cache, virtual_address & 0x1fff };

			// pages 6-7: boot space pages 0-1
		case 0x6000: return { &m_memory[ST5].cache, virtual_address & 0x1fff };
		case 0x7000: return { &m_memory[ST5].cache, virtual_address & 0x1fff };
		}
	}

	// if not in mapped mode, use unmapped system tag
	if ((ssw & SSW_M) == 0)
		return { &m_memory[get_ust_space()].cache, virtual_address };

	// get the page table entry
	pte_t pte = get_pte(virtual_address, user);

	// check for page faults
	if (pte.entry & PTE_F)
	{
		if (!machine().side_effects_disabled())
		{
			LOG("%s page fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
				mode == EXECUTE ? "instruction" : "data",
				virtual_address, ssw, pte.entry, machine().describe_context());

			set_fault(virtual_address, mode == EXECUTE ? EXCEPTION_I_PAGE_FAULT : EXCEPTION_D_PAGE_FAULT);
		}

		return { nullptr, 0 };
	}

	// check for protection level faults
	if (!machine().side_effects_disabled())
	{
		if ((mode == EXECUTE) && !get_access(mode, pte.entry, ssw))
		{
			LOGMASKED(LOG_ACCESS, "execute protection fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
				virtual_address, ssw, pte.entry, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_I_EXECUTE_PROTECT_FAULT);

			return { nullptr, 0 };
		}

		if ((mode & READ) && !get_access(READ, pte.entry, ssw))
		{
			LOGMASKED(LOG_ACCESS, "read protection fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
				virtual_address, ssw, pte.entry, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_D_READ_PROTECT_FAULT);

			return { nullptr, 0 };
		}

		if ((mode & WRITE) && !get_access(WRITE, pte.entry, ssw))
		{
			LOGMASKED(LOG_ACCESS, "write protection fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
				virtual_address, ssw, pte.entry, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_D_WRITE_PROTECT_FAULT);

			return { nullptr, 0 };
		}
	}

	// set pte referenced and dirty flags
	if ((mode & WRITE) && !(pte.entry & PTE_D))
		m_memory[ST0].cache.write_dword(pte.address, pte.entry | PTE_D | PTE_R);
	else if (!(pte.entry & PTE_R))
		m_memory[ST0].cache.write_dword(pte.address, pte.entry | PTE_R);

	// translate the address
	LOGMASKED(LOG_DTU, "%s address translated 0x%08x\n", mode == EXECUTE ? "instruction" : "data",
		(pte.entry & ~CAMMU_PAGE_MASK) | (virtual_address & CAMMU_PAGE_MASK));

	// return the system tag and translated address
	return { &m_memory[(pte.entry & PTE_ST) >> ST_SHIFT].cache, (pte.entry & ~CAMMU_PAGE_MASK) | (virtual_address & CAMMU_PAGE_MASK) };
}

cammu_c3_device::tlb_line_t &cammu_c3_device::tlb_lookup(const bool user, const u32 virtual_address, const access_type mode)
{
	const u8 set = (virtual_address >> 12) & 0x3f;
	tlb_set_t &tlb_set = m_tlb[set];

	// check w compartment
	if ((tlb_set.w.va & TLB_VA_VA) == (virtual_address & TLB_VA_VA) && (((user && (tlb_set.w.va & TLB_VA_UV)) || (!user && (tlb_set.w.va & TLB_VA_SV)))))
	{
		LOGMASKED(LOG_TLB, "tlb_lookup 0x%08x set %2d line W hit 0x%08x\n", virtual_address, set, tlb_set.w.ra);

		// mark x line least recently used
		tlb_set.u = true;

		return tlb_set.w;
	}

	// check x compartment
	if ((tlb_set.x.va & TLB_VA_VA) == (virtual_address & TLB_VA_VA) && (((user && (tlb_set.x.va & TLB_VA_UV))) || (!user && (tlb_set.x.va & TLB_VA_SV))))
	{
		LOGMASKED(LOG_TLB, "tlb_lookup 0x%08x set %2d line X hit 0x%08x\n", virtual_address, set, tlb_set.x.ra);

		// mark w line least recently used
		tlb_set.u = false;

		return tlb_set.x;
	}

	// return the least recently used line
	if (tlb_set.u)
	{
		LOGMASKED(LOG_TLB, "tlb_lookup 0x%08x set %2d line X miss\n", virtual_address, set);

		tlb_set.u = false;
		tlb_set.x.ra &= ~TLB_RA_R;

		return tlb_set.x;
	}
	else
	{
		LOGMASKED(LOG_TLB, "tlb_lookup 0x%08x set %2d line W miss\n", virtual_address, set);

		tlb_set.u = true;
		tlb_set.w.ra &= ~TLB_RA_R;

		return tlb_set.w;
	}
}

cammu_device::translated_t cammu_c3_device::translate_address(const u32 ssw, const u32 virtual_address, const access_size size, const access_type mode)
{
	// get effective user/supervisor mode
	const bool user = (mode == EXECUTE) ? (ssw & SSW_U) : (ssw & (SSW_U | SSW_UU));

	// check for alignment faults
	if (!machine().side_effects_disabled() && get_alignment())
	{
		if ((mode == EXECUTE && (virtual_address & 0x1)) || (mode != EXECUTE && virtual_address & (size - 1)))
		{
			set_fault(virtual_address, mode == EXECUTE ? EXCEPTION_I_ALIGNMENT_FAULT : EXCEPTION_D_ALIGNMENT_FAULT);

			return { nullptr, 0 };
		}
	}

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if (!user && (virtual_address & ~0x7fff) == 0)
	{
		switch (virtual_address & 0x7000)
		{
			// pages 0-3: main space pages 0-3
		case 0x0000: return { &m_memory[ST1].cache, virtual_address & 0x3fff };
		case 0x1000: return { &m_memory[ST2].cache, virtual_address & 0x3fff };
		case 0x2000: return { &m_memory[ST3].cache, virtual_address & 0x3fff };
		case 0x3000: return { &m_memory[ST3].cache, virtual_address & 0x3fff };

			// pages 4-5: i/o space pages 0-1
		case 0x4000: return { &m_memory[ST4].cache, virtual_address & 0x1fff };
		case 0x5000: return { &m_memory[ST4].cache, virtual_address & 0x1fff };

			// pages 6-7: boot space pages 0-1
		case 0x6000: return { &m_memory[ST5].cache, virtual_address & 0x1fff };
		case 0x7000: return { &m_memory[ST5].cache, virtual_address & 0x1fff };
		}
	}

	// if not in mapped mode, use unmapped system tag
	if ((ssw & SSW_M) == 0)
		return { &m_memory[get_ust_space()].cache, virtual_address };

	// check translation lookaside buffer
	tlb_line_t &tlbl = tlb_lookup(user, virtual_address, mode);

	pte_t pte = { PTE_F, 0 };

	// handle translation lookaside buffer miss
	if (!(tlbl.ra & TLB_RA_R))
	{
		// get the page table entry
		pte = get_pte(virtual_address, user);

		// check for page faults
		if (pte.entry & PTE_F)
		{
			if (!machine().side_effects_disabled())
			{
				LOG("%s page fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
					mode == EXECUTE ? "instruction" : "data",
					virtual_address, ssw, pte.entry, machine().describe_context());

				set_fault(virtual_address, mode == EXECUTE ? EXCEPTION_I_PAGE_FAULT : EXCEPTION_D_PAGE_FAULT);
			}

			return { nullptr, 0 };
		}

		// update tlb line from page table entry
		// FIXME: not sure if user/supervisor valid follow actual or effective mode?
		tlbl.va = (virtual_address & TLB_VA_VA) | (user ? TLB_VA_UV : TLB_VA_SV);
		tlbl.ra = pte.entry;
	}

	// check protection level
	if (!machine().side_effects_disabled())
	{
		if ((mode == EXECUTE) && !get_access(EXECUTE, tlbl.ra, ssw))
		{
			LOGMASKED(LOG_ACCESS, "execute protection fault address 0x%08x ssw 0x%08x (%s)\n",
				virtual_address, ssw, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_I_EXECUTE_PROTECT_FAULT);

			return { nullptr, 0 };
		}
		if ((mode & READ) && !get_access(READ, tlbl.ra, ssw))
		{
			LOGMASKED(LOG_ACCESS, "read protection fault address 0x%08x ssw 0x%08x (%s)\n",
				virtual_address, ssw, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_D_READ_PROTECT_FAULT);

			return { nullptr, 0 };
		}
		if ((mode & WRITE) && !get_access(WRITE, tlbl.ra, ssw))
		{
			LOGMASKED(LOG_ACCESS, "write protection fault address 0x%08x ssw 0x%08x (%s)\n",
				virtual_address, ssw, machine().describe_context());

			set_fault(virtual_address, EXCEPTION_D_WRITE_PROTECT_FAULT);

			return { nullptr, 0 };
		}
	}

	// update dirty flag
	if ((mode & WRITE) && !(tlbl.ra & TLB_RA_D))
	{
		// fetch the page table entry if needed
		if (pte.entry & PTE_F)
			pte = get_pte(virtual_address, user);

		// set page table entry dirty flag
		if (!(pte.entry & PTE_D))
		{
			pte.entry |= PTE_D | PTE_R;
			m_memory[ST0].cache.write_dword(pte.address, pte.entry);
		}

		tlbl.ra |= TLB_RA_D | TLB_RA_R;
	}

	// update referenced flag
	if (!(tlbl.ra & TLB_RA_R))
	{
		// fetch the page table entry if needed
		if (pte.entry & PTE_F)
			pte = get_pte(virtual_address, user);

		// set page table entry referenced flag
		if (!(pte.entry & PTE_R))
		{
			pte.entry |= PTE_R;
			m_memory[ST0].cache.write_dword(pte.address, pte.entry);
		}

		tlbl.ra |= TLB_RA_R;
	}

	// return the system tag and translated address
	LOGMASKED(LOG_DTU, "%s address translated 0x%08x\n", mode == EXECUTE ? "instruction" : "data",
		(tlbl.ra & TLB_RA_RA) | (virtual_address & CAMMU_PAGE_MASK));

	if (tlbl.ra & 0x800)
		return { &m_memory[(tlbl.ra & TLB_RA_ST) >> ST_SHIFT].cache, (tlbl.ra & TLB_RA_RA) | (virtual_address & CAMMU_PAGE_MASK) };
	else
		return { &tlbl.cache, (tlbl.ra & TLB_RA_RA) | (virtual_address & CAMMU_PAGE_MASK) };
}

// return the page table entry for a given virtual address
cammu_device::pte_t cammu_device::get_pte(const u32 va, const bool user)
{
	// get page table directory origin from user or supervisor pdo register
	const u32 pdo = get_pdo(user);

	// get page table directory index from top 12 bits of virtual address
	const u32 ptdi = (va & VA_PTDI) >> 20;

	// fetch page table directory entry
	const u32 ptde = m_memory[ST0].cache.read_dword(pdo | ptdi);

	LOGMASKED(LOG_DTU, "get_pte pdo 0x%08x ptdi 0x%08x ptde 0x%08x\n", pdo, ptdi, ptde);

	// check for page table directory entry fault
	if (ptde & PTDE_F)
		return { PTE_F, pdo | ptdi };

	// get the page table origin from the page table directory entry
	const u32 pto = ptde & PTDE_PTO;

	// get the page table index from the middle 12 bits of the virtual address
	const u32 pti = (va & VA_PTI) >> 10;

	// fetch page table entry
	pte_t pte = { m_memory[ST0].cache.read_dword(pto | pti), pto | pti };

	LOGMASKED(LOG_DTU, "get_pte pto 0x%08x pti 0x%08x pte 0x%08x\n", pto, pti, pte.entry);

	// check for page table entry fault
	if (!(pte.entry & PTE_F))
		LOGMASKED(LOG_DTU, "get_pte address 0x%08x pte 0x%08x (%s)\n", va, pte.entry, machine().describe_context());

	return pte;
}

bool cammu_c4_device::get_access(const access_type mode, const u32 pte, const u32 ssw) const
{
	switch (mode)
	{
	case READ: return pte & 0x20;
	case WRITE: return pte & 0x10;
	case EXECUTE: return pte & 0x08;

	default: return false;
	}
}

bool cammu_c3_device::get_access(const access_type mode, const u32 pte, const u32 ssw) const
{
	const u8 pl = (pte & PTE_PL) >> 3;

	// special case for user data mode
	if ((mode != EXECUTE) && !(ssw & SSW_U) && (ssw & SSW_UU))
		return protection_matrix[(ssw & SSW_KU) ? 2 : 3][pl] & mode;
	else
		return protection_matrix[((ssw ^ SSW_K) & (SSW_U | SSW_K)) >> 29][pl] & mode;
}

// C100/C300 CAMMU protection level matrix
const u8 cammu_c3_device::protection_matrix[4][16] =
{
	{ RW,  RW,  RW,  RW,  RW,  RW,  RW,  RWE, RE,  R,   R,   R,   N,   N,   N,   N },
	{ N,   RW,  RW,  RW,  RW,  RW,  R,   RWE, N,   RE,  R,   R,   RE,  N,   N,   N },
	{ N,   N,   RW,  RW,  RW,  R,   R,   RWE, N,   N,   RE,  RE,  N,   RE,  N,   N },
	{ N,   N,   N,   RW,  R,   R,   R,   RWE, N,   N,   N,   RE,  RE,  N,   RE,  N }
};

void cammu_c3_device::reset_w(const u32 data)
{
	// translation lookaside buffer reset operations
	if (data & (RESET_RSV | RESET_RUV | RESET_RD | RESET_RR))
	{
		LOGMASKED(LOG_TLB, "reset_w%s%s%s%s (%s)\n",
			(data & RESET_RSV) ? " RSV" : "",
			(data & RESET_RUV) ? " RUV" : "",
			(data & RESET_RD) ? " RD" : "",
			(data & RESET_RR) ? " RR" : "",
			machine().describe_context());

		const u32 va_mask = ((data & RESET_RSV) ? TLB_VA_SV : 0) | ((data & RESET_RUV) ? TLB_VA_UV : 0);
		const u32 ra_mask = ((data & RESET_RD) ? TLB_RA_D : 0) | ((data & RESET_RR) ? TLB_RA_R : 0);

		for (tlb_set_t &tlb_set : m_tlb)
		{
			tlb_set.w.va &= ~va_mask;
			tlb_set.w.ra &= ~ra_mask;
			tlb_set.x.va &= ~va_mask;
			tlb_set.x.ra &= ~ra_mask;
		}
	}
}

u32 cammu_c3_device::tlb_r(const u8 address) const
{
	const u8 set = address >> 2;
	u32 result = 0;

	switch (address & 0x3)
	{
	case 0x0: result = m_tlb[set].w.ra | (m_tlb[set].u ? TLB_RA_U : 0); break;
	case 0x1: result = m_tlb[set].w.va; break;
	case 0x2: result = m_tlb[set].x.ra | (m_tlb[set].u ? TLB_RA_U : 0); break;
	case 0x3: result = m_tlb[set].x.va; break;
	}

	LOGMASKED(LOG_TLB, "tlb_r set %2d line %c %s 0x%08x (%s)\n",
		set, (address & 0x2) ? 'X' : 'W', (address & 0x1) ? "va" : "ra",
		result, machine().describe_context());

	return result;
}

void cammu_c3_device::tlb_w(const u8 address, const u32 data)
{
	const u32 mem_mask = ~TLB_RA_U;
	const u8 set = address >> 2;

	LOGMASKED(LOG_TLB, "tlb_w set %2d line %c %s 0x%08x (%s)\n",
		set, (address & 0x2) ? 'X' : 'W', (address & 0x1) ? "va" : "ra",
		data, machine().describe_context());

	switch (address & 0x3)
	{
	case 0x0: COMBINE_DATA(&m_tlb[set].w.ra); break;
	case 0x1: COMBINE_DATA(&m_tlb[set].w.va); break;
	case 0x2: COMBINE_DATA(&m_tlb[set].x.ra); break;
	case 0x3: COMBINE_DATA(&m_tlb[set].x.va); break;
	}
}

u32 cammu_c3_device::cammu_r(const u32 address)
{
	switch (address & CAMMU_SELECT)
	{
	case CAMMU_D_TLB:
		return tlb_r(address);

	case CAMMU_D_REG:
		switch (address & 0xff)
		{
		case CAMMU_REG_SPDO: return s_pdo_r();
		case CAMMU_REG_UPDO: return u_pdo_r();
		case CAMMU_REG_FAULT: return fault_r();
		case CAMMU_REG_CONTROL: return control_r();
		}
		break;

	case CAMMU_I_TLB:
		return m_linked[1]->tlb_r(address);

	case CAMMU_I_REG:
		switch (address & 0xff)
		{
		case CAMMU_REG_SPDO: return m_linked[1]->s_pdo_r();
		case CAMMU_REG_UPDO: return m_linked[1]->u_pdo_r();
		case CAMMU_REG_FAULT: return m_linked[1]->fault_r();
		case CAMMU_REG_CONTROL: return m_linked[1]->control_r();
		}
		break;
	}

	LOG("cammu_r unknown address 0x%08x\n", address);
	return 0;
}

void cammu_c3_device::cammu_w(const u32 address, const u32 data)
{
	switch (address & CAMMU_SELECT)
	{
	case CAMMU_D_TLB:
		tlb_w(address, data);
		break;

	case CAMMU_D_REG:
		switch (address & 0xff)
		{
		case CAMMU_REG_SPDO: s_pdo_w(data); break;
		case CAMMU_REG_UPDO: u_pdo_w(data); break;
		case CAMMU_REG_FAULT: fault_w(data); break;
		case CAMMU_REG_CONTROL: control_w(data); break;
		case CAMMU_REG_RESET: reset_w(data); break;
		default:
			break;
		}
		break;

	case CAMMU_I_TLB:
		m_linked[1]->tlb_w(address, data);
		break;

	case CAMMU_I_REG:
		switch (address & 0xff)
		{
		case CAMMU_REG_SPDO: m_linked[1]->s_pdo_w(data); break;
		case CAMMU_REG_UPDO: m_linked[1]->u_pdo_w(data); break;
		case CAMMU_REG_FAULT: m_linked[1]->fault_w(data); break;
		case CAMMU_REG_CONTROL: m_linked[1]->control_w(data); break;
		case CAMMU_REG_RESET: m_linked[1]->reset_w(data); break;
		default:
			break;
		}
		break;

	case CAMMU_G_TLB:
		for (cammu_c3_device *cammu : m_linked)
			cammu->tlb_w(address, data);
		break;

	case CAMMU_G_REG:
		for (cammu_c3_device *cammu : m_linked)
			switch (address & 0xff)
			{
			case CAMMU_REG_SPDO: cammu->s_pdo_w(data); break;
			case CAMMU_REG_UPDO: cammu->u_pdo_w(data); break;
			case CAMMU_REG_FAULT: cammu->fault_w(data); break;
			case CAMMU_REG_CONTROL: cammu->control_w(data); break;
			case CAMMU_REG_RESET: cammu->reset_w(data); break;
			default:
				break;
		}
		break;

	default:
		LOG("cammu_w unknown address 0x%08x data 0x%08x\n", address, data);
		break;
	}
}
