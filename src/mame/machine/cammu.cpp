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
 *   - fault register values
 *   - c3 protection faults
 *   - hard-wired and dynamic tlb
 *   - cache
 *   - bus errors
 */

#include "emu.h"
#include "cammu.h"

#define LOG_GENERAL (1U << 0)
#define LOG_ACCESS  (1U << 1)
#define LOG_DTU     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_ACCESS | LOG_DTU)
#include "logmacro.h"

// each variant of the cammu has different registers and a different addressing map
ADDRESS_MAP_START(cammu_c4t_device::map)
	AM_RANGE(0x008, 0x00b) AM_READWRITE(ram_line_r, ram_line_w)
	AM_RANGE(0x010, 0x013) AM_READWRITE(s_pdo_r, s_pdo_w)
	AM_RANGE(0x018, 0x01b) AM_READWRITE(u_pdo_r, u_pdo_w)
	AM_RANGE(0x020, 0x023) AM_READWRITE(htlb_offset_r, htlb_offset_w)
	AM_RANGE(0x028, 0x02b) AM_READWRITE(i_fault_r, i_fault_w)
	AM_RANGE(0x030, 0x033) AM_READWRITE(fault_address_1_r, fault_address_1_w)
	AM_RANGE(0x038, 0x03b) AM_READWRITE(fault_address_2_r, fault_address_2_w)
	AM_RANGE(0x040, 0x043) AM_READWRITE(fault_data_1_lo_r, fault_data_1_lo_w)
	AM_RANGE(0x048, 0x04b) AM_READWRITE(fault_data_1_hi_r, fault_data_1_hi_w)
	AM_RANGE(0x050, 0x053) AM_READWRITE(fault_data_2_lo_r, fault_data_2_lo_w)
	AM_RANGE(0x058, 0x05b) AM_READWRITE(fault_data_2_hi_r, fault_data_2_hi_w)
	AM_RANGE(0x060, 0x063) AM_READWRITE(c4_bus_poll_r, c4_bus_poll_w)
	AM_RANGE(0x068, 0x06b) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x070, 0x073) AM_READWRITE(bio_control_r, bio_control_w)
	AM_RANGE(0x078, 0x07b) AM_READWRITE(bio_address_tag_r, bio_address_tag_w)

	AM_RANGE(0x100, 0x103) AM_READWRITE(cache_data_lo_r, cache_data_lo_w)
	AM_RANGE(0x104, 0x107) AM_READWRITE(cache_data_hi_r, cache_data_hi_w)
	AM_RANGE(0x108, 0x10b) AM_READWRITE(cache_cpu_tag_r, cache_cpu_tag_w)
	AM_RANGE(0x10c, 0x10f) AM_READWRITE(cache_system_tag_valid_r, cache_system_tag_valid_w)
	AM_RANGE(0x110, 0x113) AM_READWRITE(cache_system_tag_r, cache_system_tag_w)
	AM_RANGE(0x118, 0x11b) AM_READWRITE(tlb_va_line_r, tlb_va_line_w)
	AM_RANGE(0x11c, 0x11f) AM_READWRITE(tlb_ra_line_r, tlb_ra_line_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(cammu_c4i_device::map)
	AM_RANGE(0x000, 0x003) AM_READWRITE(reset_r, reset_w)
	AM_RANGE(0x010, 0x013) AM_READWRITE(s_pdo_r, s_pdo_w)
	AM_RANGE(0x018, 0x01b) AM_READWRITE(u_pdo_r, u_pdo_w)
	AM_RANGE(0x020, 0x023) AM_READWRITE(clr_s_data_tlb_r, clr_s_data_tlb_w)
	AM_RANGE(0x028, 0x02b) AM_READWRITE(clr_u_data_tlb_r, clr_u_data_tlb_w)
	AM_RANGE(0x030, 0x033) AM_READWRITE(clr_s_insn_tlb_r, clr_s_insn_tlb_w)
	AM_RANGE(0x038, 0x03b) AM_READWRITE(clr_u_insn_tlb_r, clr_u_insn_tlb_w)

	AM_RANGE(0x068, 0x06b) AM_READWRITE(control_r, control_w)

	AM_RANGE(0x080, 0x083) AM_READWRITE(test_data_r, test_data_w)
	AM_RANGE(0x088, 0x08b) AM_READWRITE(i_fault_r, i_fault_w)
	AM_RANGE(0x090, 0x093) AM_READWRITE(fault_address_1_r, fault_address_1_w)
	AM_RANGE(0x098, 0x09b) AM_READWRITE(fault_address_2_r, fault_address_2_w)
	AM_RANGE(0x0a0, 0x0a3) AM_READWRITE(fault_data_1_lo_r, fault_data_1_lo_w)
	AM_RANGE(0x0a8, 0x0ab) AM_READWRITE(fault_data_1_hi_r, fault_data_1_hi_w)
	AM_RANGE(0x0b0, 0x0b3) AM_READWRITE(fault_data_2_lo_r, fault_data_2_lo_w)
	AM_RANGE(0x0b8, 0x0bb) AM_READWRITE(fault_data_2_hi_r, fault_data_2_hi_w)
	AM_RANGE(0x0c0, 0x0c3) AM_READWRITE(test_address_r, test_address_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(cammu_c3_device::map)
	AM_RANGE(0x000, 0x0ff) AM_NOP // tlb
	AM_RANGE(0x104, 0x107) AM_READWRITE(s_pdo_r, s_pdo_w)
	AM_RANGE(0x108, 0x10b) AM_READWRITE(u_pdo_r, u_pdo_w)
	AM_RANGE(0x110, 0x113) AM_READWRITE(fault_r, fault_w)
	AM_RANGE(0x140, 0x143) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x180, 0x183) AM_READWRITE(reset_r, reset_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(cammu_c3_device::map_global)
	AM_RANGE(0x000, 0x0ff) AM_NOP // global tlb
	AM_RANGE(0x104, 0x107) AM_WRITE(g_s_pdo_w)
	AM_RANGE(0x108, 0x10b) AM_WRITE(g_u_pdo_w)
	AM_RANGE(0x110, 0x113) AM_WRITE(g_fault_w)
	AM_RANGE(0x140, 0x143) AM_WRITE(g_control_w)
	AM_RANGE(0x180, 0x183) AM_WRITE(g_reset_w)
ADDRESS_MAP_END

DEFINE_DEVICE_TYPE(CAMMU_C4T, cammu_c4t_device, "c4t", "C4E/C4T CAMMU")
DEFINE_DEVICE_TYPE(CAMMU_C4I, cammu_c4i_device, "c4i", "C4I CAMMU")
DEFINE_DEVICE_TYPE(CAMMU_C3,  cammu_c3_device,  "c3",  "C1/C3 CAMMU")

cammu_c4t_device::cammu_c4t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_c4_device(mconfig, CAMMU_C4T, tag, owner, clock)
{
}

cammu_c4i_device::cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_c4_device(mconfig, CAMMU_C4I, tag, owner, clock)
{
}

cammu_c4_device::cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, type, tag, owner, clock)
{
}

cammu_c3_device::cammu_c3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, CAMMU_C3, tag, owner, clock)
	, m_control(CID_C3)
	, m_linked{ this }
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
	save_item(NAME(m_reset));
}

void cammu_c3_device::device_reset()
{
	cammu_device::device_reset();

	m_control = (m_control & CNTL_CID) | (CNTL_ATE | UST_3 | CNTL_EWIR | CNTL_EWIW | CNTL_EWCW | CNTL_EP);
	m_reset = 0;
}

void cammu_device::set_spaces(std::vector<address_space *> spaces)
{
	assert_always(spaces.size() == 8, "exactly 8 address space pointers are required");

	for (int i = 0; i < spaces.size(); i++)
		m_space[i] = spaces[i];
}

cammu_device::translated_t cammu_device::translate_address(const u32 ssw, const u32 virtual_address, const access_size size, const access_type mode)
{
	// get effective user/supervisor mode
	const bool user = mode == EXECUTE ? ssw & SSW_U : ssw & (SSW_U | SSW_UU);

	// check for alignment faults
	if (get_alignment() && !machine().side_effects_disabled())
	{
		if ((mode == EXECUTE && (virtual_address & 0x1)) || (mode != EXECUTE && virtual_address & (size - 1)))
		{
			set_fault_address(virtual_address);
			m_exception_func(mode == EXECUTE ? EXCEPTION_I_ALIGNMENT_FAULT : EXCEPTION_D_ALIGNMENT_FAULT);

			return { nullptr, 0 };
		}
	}

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if (!user && (virtual_address & ~0x7fff) == 0)
	{
		switch (virtual_address & 0x7000)
		{
			// pages 0-3: main space pages 0-3
		case 0x0000: return { m_space[ST1], virtual_address & 0x3fff };
		case 0x1000: return { m_space[ST2], virtual_address & 0x3fff };
		case 0x2000: return { m_space[ST3], virtual_address & 0x3fff };
		case 0x3000: return { m_space[ST3], virtual_address & 0x3fff };

			// pages 4-5: i/o space pages 0-1
		case 0x4000: return { m_space[ST4], virtual_address & 0x1fff };
		case 0x5000: return { m_space[ST4], virtual_address & 0x1fff };

			// pages 6-7: boot space pages 0-1
		case 0x6000: return { m_space[ST5], virtual_address & 0x1fff };
		case 0x7000: return { m_space[ST5], virtual_address & 0x1fff };
		}
	}

	// if not in mapped mode, use unmapped system tag
	if ((ssw & SSW_M) == 0)
		return { m_space[get_ust_space()], virtual_address };

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

			set_fault_address(virtual_address);
			m_exception_func(mode == EXECUTE ? EXCEPTION_I_PAGE_FAULT : EXCEPTION_D_PAGE_FAULT);
		}

		return { nullptr, 0 };
	}

	// check for protection level faults
	if (!machine().side_effects_disabled() && !get_access(mode, pte.entry, ssw))
	{
		LOG("%s protection fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
			mode == EXECUTE ? "execute" : mode == READ ? "read" : "write",
			virtual_address, ssw, pte.entry, machine().describe_context());

		set_fault_address(virtual_address);
		m_exception_func(
			mode == EXECUTE ? EXCEPTION_I_EXECUTE_PROTECT_FAULT :
			mode == READ ? EXCEPTION_D_READ_PROTECT_FAULT :
			EXCEPTION_D_WRITE_PROTECT_FAULT);

		return { nullptr, 0 };
	}

	// set pte referenced and dirty flags
	if (mode & WRITE && !(pte.entry & PTE_D))
		m_space[ST0]->write_dword(pte.address, pte.entry | PTE_D | PTE_R);
	else if (!(pte.entry & PTE_R))
		m_space[ST0]->write_dword(pte.address, pte.entry | PTE_R);

	// translate the address
	LOGMASKED(LOG_DTU, "%s address translated 0x%08x\n", mode == EXECUTE ? "instruction" : "data",
		(pte.entry & ~CAMMU_PAGE_MASK) | (virtual_address & CAMMU_PAGE_MASK));

	// return the system tag and translated address
	return { m_space[system_tag_t((pte.entry & PTE_ST) >> PTE_ST_SHIFT)], (pte.entry & ~CAMMU_PAGE_MASK) | (virtual_address & CAMMU_PAGE_MASK) };
}

// return the page table entry for a given virtual address
cammu_device::pte_t cammu_device::get_pte(const u32 va, const bool user)
{
	// get page table directory origin from user or supervisor pdo register
	const u32 pdo = get_pdo(user);

	// get page table directory index from top 12 bits of virtual address
	const u32 ptdi = (va & VA_PTDI) >> 20;

	// fetch page table directory entry
	const u32 ptde = m_space[ST0]->read_dword(pdo | ptdi);

	LOGMASKED(LOG_DTU, "get_pte pdo 0x%08x ptdi 0x%08x ptde 0x%08x\n", pdo, ptdi, ptde);

	// check for page table directory entry fault
	if (ptde & PTDE_F)
		return { PTE_F, pdo | ptdi };

	// get the page table origin from the page table directory entry
	const u32 pto = ptde & PTDE_PTO;

	// get the page table index from the middle 12 bits of the virtual address
	const u32 pti = (va & VA_PTI) >> 10;

	// fetch page table entry
	pte_t pte = { m_space[ST0]->read_dword(pto | pti), pto | pti };

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
	case RMW: return (pte & 0x30) == 0x30;
	case EXECUTE: return pte & 0x08;
	}

	return false;
}

bool cammu_c3_device::get_access(const access_type mode, const u32 pte, const u32 ssw) const
{
	// FIXME: logic is not correct yet
	return true;

	const u8 column = (mode == EXECUTE ? i_cammu_column : d_cammu_column)[(ssw & SSW_PL) >> 9];
	const u8 access = cammu_matrix[column][(pte & PTE_PL) >> 3];

	switch (mode)
	{
	case READ: return access & R;
	case WRITE: return access & W;
	case RMW: return (access & (R | W)) == (R | W);
	case EXECUTE: return access & E;
	}

	return false;
}

// C100/C300 CAMMU protection level matrix
const u8 cammu_c3_device::i_cammu_column[] = { 1, 1, 0, 0, 1, 1, 0, 0, 3, 3, 2, 2, 3, 3, 2, 2 };
const u8 cammu_c3_device::d_cammu_column[] = { 1, 1, 0, 0, 3, 2, 3, 2, 3, 3, 2, 2, 3, 3, 2, 2 };

const cammu_c3_device::c3_access_t cammu_c3_device::cammu_matrix[][16] =
{
	{ RW,  RW,  RW,  RW,  RW,  RW,  RW,  RWE, RE,  R,   R,   R,   N,   N,   N,   N },
	{ N,   RW,  RW,  RW,  RW,  RW,  R,   RWE, N,   RE,  R,   R,   RE,  N,   N,   N },
	{ N,   N,   RW,  RW,  RW,  R,   R,   RWE, N,   N,   RE,  RE,  N,   RE,  N,   N },
	{ N,   N,   N,   RW,  R,   R,   R,   RWE, N,   N,   N,   RE,  RE,  N,   RE,  N }
};
