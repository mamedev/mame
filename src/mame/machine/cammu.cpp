// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Fairchild/Intergraph Cache and Memory Management Unit (CAMMU) designed for use with the CLIPPER CPU family.
 *
 * Primary reference: http://bitsavers.trailing-edge.com/pdf/fairchild/clipper/CLIPPER%20C300%2032-Bit%20Compute%20Engine.pdf
 * Another reference: http://www.eecs.berkeley.edu/Pubs/TechRpts/1986/CSD-86-289.pdf
 *
 * This implementation is currently at a very early stage, and is only sufficient to handle the bare minimum of boot/diagnostic code.
 *
 * TODO
 *   - almost everything
 *   - refactor hardware tlb
 *   - faults
 *   - tlb
 *   - cache
 *   - bus errors
 */

#include "emu.h"

#define VERBOSE 0
#define DTU 1 // enable preliminary/incomplete address translation

#include "cammu.h"

// each variant of the cammu has different registers and a different addressing map
DEVICE_ADDRESS_MAP_START(map, 32, cammu_c4t_device)
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

DEVICE_ADDRESS_MAP_START(map, 32, cammu_c4i_device)
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

DEVICE_ADDRESS_MAP_START(map, 32, cammu_c3_device)
	// the first AM_NOP in each range is in fact the TLB in the C3 CAMMU

	AM_RANGE(0x800, 0x8ff) AM_NOP
	AM_RANGE(0x904, 0x907) AM_READWRITE(d_s_pdo_r, d_s_pdo_w)
	AM_RANGE(0x908, 0x90b) AM_READWRITE(d_u_pdo_r, d_u_pdo_w)
	AM_RANGE(0x910, 0x913) AM_READWRITE(d_fault_r, d_fault_w)
	AM_RANGE(0x940, 0x943) AM_READWRITE(d_control_r, d_control_w)
	AM_RANGE(0x980, 0x983) AM_READWRITE(d_reset_r, d_reset_w)

	AM_RANGE(0xa00, 0xaff) AM_NOP
	AM_RANGE(0xb04, 0xb07) AM_READWRITE(i_s_pdo_r, i_s_pdo_w)
	AM_RANGE(0xb08, 0xb0b) AM_READWRITE(i_u_pdo_r, i_u_pdo_w)
	AM_RANGE(0xb10, 0xb13) AM_READWRITE(i_fault_r, i_fault_w)
	AM_RANGE(0xb40, 0xb43) AM_READWRITE(i_control_r, i_control_w)
	AM_RANGE(0xb80, 0xb83) AM_READWRITE(i_reset_r, i_reset_w)

	AM_RANGE(0xc00, 0xcff) AM_NOP
	AM_RANGE(0xd04, 0xd07) AM_WRITE(g_s_pdo_w)
	AM_RANGE(0xd08, 0xd0b) AM_WRITE(g_u_pdo_w)
	AM_RANGE(0xd10, 0xd13) AM_WRITE(g_fault_w)
	AM_RANGE(0xd40, 0xd43) AM_WRITE(g_control_w)
	AM_RANGE(0xd80, 0xd83) AM_WRITE(g_reset_w)
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
{
}

cammu_device::cammu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this),
	m_main_space_config("main", ENDIANNESS_LITTLE, 32, 32, 0),
	m_io_space_config("io", ENDIANNESS_LITTLE, 32, 32, 0),
	m_boot_space_config("boot", ENDIANNESS_LITTLE, 32, 32, 0),
	m_main_space(nullptr),
	m_io_space(nullptr),
	m_boot_space(nullptr),
	m_ssw_func(*this)
{
}

void cammu_device::device_start()
{
	m_ssw_func.resolve();

	m_main_space = &space(0);
	m_io_space = &space(1);
	m_boot_space = &space(2);
}

void cammu_device::device_reset()
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

READ32_MEMBER(cammu_device::insn_r)
{
	u32 ssw = m_ssw_func();
	u32 va = offset << 2;

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if ((ssw & 0x40000000) == 0 && (va & ~0x7fff) == 0)
	{
		switch (va & 0xf000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			return m_main_space->read_dword(va, mem_mask);

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			return m_io_space->read_dword(va & 0x1fff, mem_mask);

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			return m_boot_space->read_dword(va & 0x1fff, mem_mask);
		}
	}

	// if not in mapped mode, default to main memory space
	if ((ssw & 0x04000000) == 0)
		return m_main_space->read_dword(va);

#if DTU
	// get the page table entry
	u32 pte = get_pte(va, ssw & 0x40000000, false);

	// translate the address
	u32 ra = (pte & 0xfffff000) | (va & 0xfff);

	// execute the read based on the system tag
	switch ((pte & 0xe00) >> 9)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_main_space->read_dword(ra, mem_mask);

	case 4:
		return m_io_space->read_dword(ra, mem_mask);

	case 5:
		return m_boot_space->read_dword(ra, mem_mask);

	case 6: // cache purge
	case 7: // main memory, slave mode
		fatalerror("system tag %d not supported %s\n", (pte & 0xe00) >> 9, machine().describe_context().c_str());
	}

	return 0;
#else
	// FIXME: currently maps addresses with upper bits 0x00 or 0x7f1 to main memory and everything else to I/O
	if ((va & 0xff000000) == 0x00000000 || (va & 0xfff00000) == 0x7f100000)
		return m_main_space->read_dword(va, mem_mask);
	else
		return m_io_space->read_dword(va, mem_mask);
#endif
}

READ32_MEMBER(cammu_device::data_r)
{
	u32 ssw = m_ssw_func();
	u32 va = offset << 2;

	// in supervisor mode (and not user data mode), the first 8 pages are always mapped via the hard-wired tlb
	if (((ssw & 0x50000000) == 0) && ((va & ~0x7fff) == 0))
	{
		switch (va & 0xf000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			return m_main_space->read_dword(va, mem_mask);

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			return m_io_space->read_dword(va & 0x1fff, mem_mask);

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			return m_boot_space->read_dword(va & 0x1fff, mem_mask);
		}
	}

	// if not in mapped mode, default to main memory space
	if ((ssw & 0x04000000) == 0)
		return m_main_space->read_dword(va);

#if DTU
	// get the page table entry
	u32 pte = get_pte(va, ssw & 0x50000000, space.spacenum() == AS_DATA);

	// translate the address
	u32 ra = (pte & 0xfffff000) | (va & 0xfff);

	// execute the read based on the system tag
	switch ((pte & 0xe00) >> 9)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_main_space->read_dword(ra, mem_mask);

	case 4:
		return m_io_space->read_dword(ra, mem_mask);

	case 5:
		return m_boot_space->read_dword(ra, mem_mask);

	case 6: // cache purge
	case 7: // main memory, slave mode
		fatalerror("data_r: system tag %d not supported at %s\n", (pte & 0xe00) >> 9, machine().describe_context().c_str());
	}

	return 0;
#else
	// FIXME: currently maps addresses with upper bits 0x00 or 0x7f1 to main memory and everything else to I/O
	if ((va & 0xff000000) == 0x00000000 || (va & 0xfff00000) == 0x7f100000)
		return m_main_space->read_dword(va, mem_mask);
	else
		return m_io_space->read_dword(va, mem_mask);
#endif
}

WRITE32_MEMBER(cammu_device::data_w)
{
	u32 ssw = m_ssw_func();
	u32 va = offset << 2;

	// in supervisor mode (and not user data mode), the first 8 pages are always mapped via the hard-wired tlb
	if (((ssw & 0x50000000) == 0) && ((va & ~0x7fff) == 0))
	{
		switch (va & 0xf000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			m_main_space->write_dword(va, data, mem_mask);
			return;

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			m_io_space->write_dword(va & 0x1fff, data, mem_mask);
			return;

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			m_boot_space->write_dword(va & 0x1fff, data, mem_mask);
			return;
		}
	}

	// if not in mapped mode, default to main memory space
	if ((ssw & 0x04000000) == 0)
	{
		m_main_space->write_dword(va, data, mem_mask);
		return;
	}

#if DTU
	// get the page table entry
	u32 pte = get_pte(va, ssw & 0x50000000, space.spacenum() == AS_DATA);

	// translate the address
	u32 ra = (pte & 0xfffff000) | (va & 0xfff);

	// execute the read based on the system tag
	switch ((pte & 0xe00) >> 9)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		m_main_space->write_dword(ra, data, mem_mask);
		break;

	case 4:
		m_io_space->write_dword(ra, data, mem_mask);
		break;

	case 5:
		m_boot_space->write_dword(ra, data, mem_mask);
		break;

	case 6: // cache purge
	case 7: // main memory, slave mode
		fatalerror("data_w: system tag %d not supported at %s\n", (pte & 0xe00) >> 9, machine().describe_context().c_str());
		break;
	}
#else
	// FIXME: currently maps addresses with upper bits 0x00 or 0x7f1 to main memory and everything else to I/O
	if ((va & 0xff000000) == 0x00000000 || (va & 0xfff00000) == 0x7f100000)
		m_main_space->write_dword(va, data, mem_mask);
	else
		m_io_space->write_dword(va, data, mem_mask);
#endif
}

u32 cammu_c4_device::get_pte(u32 va, int user, bool data)
{
	u32 tlb_index = (user ? 2 : 0) + (data ? 1 : 0);
	if ((va & 0xfffff000) != m_tlb[tlb_index].va)
	{
		// return the page table entry for a given virtual address
		u32 pdo = user ? m_u_pdo : m_s_pdo;

		u32 pto = m_main_space->read_dword(pdo | (va & 0xffc00000) >> 20);
		if (pto & 0x1)
			fatalerror("can't deal with pto faults va 0x%08x %s\n", va, machine().describe_context().c_str());

		u32 pte = m_main_space->read_dword((pto & 0xfffff000) | (va & 0x003ff000) >> 10);
		if (pte & 0x1)
			fatalerror("can't deal with pte faults va 0x%08x %s\n", va, machine().describe_context().c_str());

		m_tlb[tlb_index].va = va & 0xfffff000;
		m_tlb[tlb_index].pte = pte;
	}

	return m_tlb[tlb_index].pte;
}

u32 cammu_c3_device::get_pte(u32 va, int user, bool data)
{
	// return the page table entry for a given virtual address
	u32 pdo = user ? (data ? m_d_u_pdo : m_i_u_pdo) : (data ? m_d_s_pdo : m_i_s_pdo);

	u32 pto = m_main_space->read_dword(pdo | (va & 0xffc00000) >> 20);
	if (pto & 0x1)
		fatalerror("can't deal with pto faults va 0x%08x %s\n", va, machine().describe_context().c_str());

	u32 pte = m_main_space->read_dword((pto & 0xfffff000) | (va & 0x003ff000) >> 10);
	if (pte & 0x1)
		fatalerror("can't deal with pte faults va 0x%08x %s\n", va, machine().describe_context().c_str());

	return pte;
}
