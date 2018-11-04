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
 *   - alignment faults
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
	: cammu_c4_device(mconfig, CAMMU_C4T, tag, owner, clock, CID_C4T)
{
}

cammu_c4i_device::cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_c4_device(mconfig, CAMMU_C4I, tag, owner, clock, CID_C4I)
{
}

cammu_c4_device::cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 cammu_id)
	: cammu_device(mconfig, type, tag, owner, clock)
	, m_control(cammu_id)
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
	, device_memory_interface(mconfig, *this)
	, m_main_space_config("main", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_io_space_config("io", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_boot_space_config("boot", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_main_space(nullptr)
	, m_io_space(nullptr)
	, m_boot_space(nullptr)
	, m_ssw_func(*this)
	, m_exception_func(*this)
{
}

device_memory_interface::space_config_vector cammu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_main_space_config),
		std::make_pair(1, &m_io_space_config),
		std::make_pair(2, &m_boot_space_config)
	};
}

void cammu_c3_device::static_add_linked(device_t &device, const char *const tag)
{
	cammu_c3_device &parent = downcast<cammu_c3_device &>(device);

	parent.m_linked.push_back(downcast<cammu_c3_device *>(parent.siblingdevice(tag)));
}

void cammu_device::device_start()
{
	m_ssw_func.resolve();
	m_exception_func.resolve();

	m_main_space = &space(0);
	m_io_space = &space(1);
	m_boot_space = &space(2);
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

READ32_MEMBER(cammu_device::read)
{
	const u32 virtual_address = (offset << 2) | (
		(mem_mask & 0x00ffffff) == 0 ? 0x3 :
		(mem_mask & 0x0000ffff) == 0 ? 0x2 :
		(mem_mask & 0x000000ff) == 0 ? 0x1 : 0x0);
	offs_t physical_address;

	LOGMASKED(LOG_ACCESS, "%s read address 0x%08x mem_mask 0x%08x (%s)\n",
		space.name(), virtual_address, mem_mask, machine().describe_context());

	address_space *physical_space = translate_address(virtual_address, space.spacenum() == AS_PROGRAM ? ACCESS_X : ACCESS_R, &physical_address);

	if (physical_space != nullptr)
		return physical_space->read_dword(physical_address, mem_mask);
	else
		return space.unmap();
}

WRITE32_MEMBER(cammu_device::write)
{
	const u32 virtual_address = (offset << 2) | (
		(mem_mask & 0x00ffffff) == 0 ? 0x3 :
		(mem_mask & 0x0000ffff) == 0 ? 0x2 :
		(mem_mask & 0x000000ff) == 0 ? 0x1 : 0x0);
	offs_t physical_address;

	LOGMASKED(LOG_ACCESS, "%s write address 0x%08x data 0x%08x mem_mask 0x%08x (%s)\n",
		space.name(), virtual_address, data, mem_mask, machine().describe_context());

	address_space *physical_space = translate_address(virtual_address, ACCESS_W, &physical_address);

	if (physical_space != nullptr)
		physical_space->write_dword(physical_address, data, mem_mask);
}

address_space *cammu_device::translate_address(const offs_t virtual_address, const access_t mode, offs_t *physical_address)
{
	// get ssw and user/supervisor mode
	const u32 ssw = m_ssw_func();
	const bool user = mode == ACCESS_X ? ssw & SSW_U : ssw & (SSW_U | SSW_UU);

	// TODO: check for alignment faults

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if (!user && (virtual_address & ~0x7fff) == 0)
	{
		switch (virtual_address & 0x7000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			*physical_address = virtual_address & 0x3fff;
			return m_main_space;

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			*physical_address = virtual_address & 0x1fff;
			return m_io_space;

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			*physical_address = virtual_address & 0x1fff;
			return m_boot_space;
		}
	}

	// if not in mapped mode, use unmapped system tag
	if ((ssw & SSW_M) == 0)
	{
		*physical_address = virtual_address;

		return get_ust_space();
	}

	// get the page table entry
	const u32 pte = get_pte(virtual_address, user);

	// check for page faults
	if (pte & PTE_F)
	{
		if (!machine().side_effect_disabled())
		{
			LOG("%s page fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
				mode == ACCESS_X ? "instruction" : "data",
				virtual_address, ssw, pte, machine().describe_context());

			set_fault_address(virtual_address);
			m_exception_func(mode == ACCESS_X ? EXCEPTION_I_PAGE_FAULT : EXCEPTION_D_PAGE_FAULT);
		}

		return nullptr;
	}

	// check for protection level faults
	if (!machine().side_effect_disabled() && !get_access(mode, pte, ssw))
	{
		LOG("%s protection fault address 0x%08x ssw 0x%08x pte 0x%08x (%s)\n",
			mode == ACCESS_X ? "execute" : mode == ACCESS_R ? "read" : "write",
			virtual_address, ssw, pte, machine().describe_context());

		set_fault_address(virtual_address);
		m_exception_func(
			mode == ACCESS_X ? EXCEPTION_I_EXECUTE_PROTECT_FAULT :
			mode == ACCESS_R ? EXCEPTION_D_READ_PROTECT_FAULT :
			EXCEPTION_D_WRITE_PROTECT_FAULT);

		return nullptr;
	}

	// translate the address
	*physical_address = (pte & ~CAMMU_PAGE_MASK) | (virtual_address & CAMMU_PAGE_MASK);
	LOGMASKED(LOG_DTU, "%s address translated 0x%08x\n", mode == ACCESS_X ? "instruction" : "data", *physical_address);

	// return the physical space based on the system tag
	switch (pte & PTE_ST)
	{
	case ST_0:
	case ST_1:
	case ST_2:
	case ST_3:
		// main memory space
		return m_main_space;

	case ST_4:
		// i/o space
		return m_io_space;

	case ST_5:
		// boot space
		return m_boot_space;

	case ST_6:
		// cache purge
	case ST_7:
		// main memory, slave mode
		if (!machine().side_effect_disabled())
			fatalerror("%s page table entry system tag %d not supported\n",
				mode == ACCESS_X ? "instruction" : "data", (pte & PTE_ST) >> 9);
		break;
	}

	return nullptr;
}

// return the page table entry for a given virtual address
u32 cammu_device::get_pte(const u32 va, const bool user)
{
	const u32 tlb_index = user ? 1 : 0;
	if ((va & ~CAMMU_PAGE_MASK) != m_tlb[tlb_index].va)
	{
		// get page table directory origin from user or supervisor pdo register
		const u32 pdo = get_pdo(user);

		// get page table directory index from top 12 bits of virtual address
		const u32 ptdi = (va & VA_PTDI) >> 20;

		// fetch page table directory entry
		const u32 ptde = m_main_space->read_dword(pdo | ptdi);

		LOGMASKED(LOG_DTU, "get_pte pdo 0x%08x ptdi 0x%08x ptde 0x%08x\n", pdo, ptdi, ptde);

		// check for page table directory entry fault
		if (ptde & PTDE_F)
			return PTE_F;

		// get the page table origin from the page table directory entry
		const u32 pto = ptde & PTDE_PTO;

		// get the page table index from the middle 12 bits of the virtual address
		const u32 pti = (va & VA_PTI) >> 10;

		// fetch page table entry
		const u32 pte = m_main_space->read_dword(pto | pti);

		LOGMASKED(LOG_DTU, "get_pte pto 0x%08x pti 0x%08x pte 0x%08x\n", pto, pti, pte);

		// check for page table entry fault
		if (pte & PTE_F)
			return PTE_F;

		// add the pte to the tlb
		m_tlb[tlb_index].va = va & ~CAMMU_PAGE_MASK;
		m_tlb[tlb_index].pte = pte;

		LOGMASKED(LOG_DTU, "get_pte address 0x%08x pte 0x%08x (%s)\n", va, pte, machine().describe_context());
	}

	return m_tlb[tlb_index].pte;
}

address_space *cammu_c4i_device::get_ust_space() const
{
	switch (m_control & CNTL_UMM)
	{
	case UMM_MM: return m_main_space;
	case UMM_MMRIO: return m_main_space; // FIXME: what determines main or i/o?
	case UMM_IO: return m_io_space;
	}

	return m_main_space;
}

bool cammu_c4_device::get_access(const access_t mode, const u32 pte, const u32 ssw) const
{
	switch (mode)
	{
	case ACCESS_R: return pte & 0x20;
	case ACCESS_W: return pte & 0x10;
	case ACCESS_X: return pte & 0x08;
	}

	return false;
}

bool cammu_c3_device::get_access(const access_t mode, const u32 pte, const u32 ssw) const
{
	// FIXME: logic is not correct yet
	return true;

	const u8 column = (mode == ACCESS_X ? i_cammu_column : d_cammu_column)[(ssw & SSW_PL) >> 9];
	const u8 access = cammu_matrix[column][(pte & PTE_PL) >> 3];

	switch (mode)
	{
	case ACCESS_R: return access & R;
	case ACCESS_W: return access & W;
	case ACCESS_X: return access & E;
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


