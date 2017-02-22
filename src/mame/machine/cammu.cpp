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
 *   - map registers
 *   - refactor hardware tlb
 *   - address translation
 *   - faults
 *   - tlb
 *   - cache
 *   - bus errors
 */

#include "emu.h"
#include "cammu.h"

#define VERBOSE 0

// each variant of the cammu has different registers and a different addressing map
// TODO: decode the cammu registers properly
DEVICE_ADDRESS_MAP_START(map, 32, cammu_c4t_device)
	AM_RANGE(0x000, 0xfff) AM_READWRITE(cammu_r, cammu_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map, 32, cammu_c4i_device)
	AM_RANGE(0x000, 0xfff) AM_READWRITE(cammu_r, cammu_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map, 32, cammu_c3_device)
	AM_RANGE(0x000, 0xfff) AM_READWRITE(cammu_r, cammu_w)
ADDRESS_MAP_END

const device_type CAMMU_C4T = &device_creator<cammu_c4t_device>;
const device_type CAMMU_C4I = &device_creator<cammu_c4i_device>;
const device_type CAMMU_C3 = &device_creator<cammu_c3_device>;

cammu_c4t_device::cammu_c4t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, CAMMU_C4T, "C4E/C4T CAMMU", tag, owner, clock, "C4T", __FILE__) { }

cammu_c4i_device::cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, CAMMU_C4I, "C4I CAMMU", tag, owner, clock, "C4I", __FILE__) { }

cammu_c3_device::cammu_c3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cammu_device(mconfig, CAMMU_C4T, "C1/C3 CAMMU", tag, owner, clock, "C3", __FILE__) { }

cammu_device::cammu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_memory_interface(mconfig, *this),
	m_main_space_config("main", ENDIANNESS_LITTLE, 32, 32, 0),
	m_io_space_config("io", ENDIANNESS_LITTLE, 32, 32, 0),
	m_boot_space_config("boot", ENDIANNESS_LITTLE, 32, 32, 0),
	m_main_space(nullptr),
	m_io_space(nullptr),
	m_boot_space(nullptr),
	m_ssw_func(*this)
{ }

void cammu_device::device_start()
{
	m_ssw_func.resolve_safe(0);

	m_main_space = &space(AS_0);
	m_io_space = &space(AS_1);
	m_boot_space = &space(AS_2);
}

void cammu_device::device_reset()
{
}

const address_space_config *cammu_device::memory_space_config (address_spacenum spacenum) const
{
	switch (spacenum)
	{
	case AS_0: return &m_main_space_config;
	case AS_1: return &m_io_space_config;
	case AS_2: return &m_boot_space_config;
	default: break;
	}

	return nullptr;
}

READ32_MEMBER(cammu_device::mmu_r)
{
	u32 ssw = m_ssw_func();
	u32 address = offset << 2;

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if ((ssw & 0x40000000) == 0 && (address & ~0x7fff) == 0)
	{
		switch (address & 0xf000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			return m_main_space->read_dword(address, mem_mask);

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			return m_io_space->read_dword(address & 0x1fff, mem_mask);

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			return m_boot_space->read_dword(address & 0x1fff, mem_mask);
		}
	}

	// FIXME: currently maps addresses with upper bits 0x00 or 0x7f1 to main memory and everything else to I/O
	if ((address & 0xff000000) == 0x00000000 || (address & 0xfff00000) == 0x7f100000)
	{
#ifdef ICACHE_ENTRIES
		// if this is an instruction fetch, check the cache first
		if (space.spacenum() == AS_PROGRAM)
		{
			if (m_icache[offset & (ICACHE_ENTRIES-1)].offset != offset)
			{
				m_icache[offset & (ICACHE_ENTRIES - 1)].offset = offset;
				m_icache[offset & (ICACHE_ENTRIES - 1)].data = m_main_space->read_dword(address, mem_mask);
			}

			return m_icache[offset & (ICACHE_ENTRIES - 1)].data;
		}
		else
#endif
			return m_main_space->read_dword(address, mem_mask);
	}
	else
		return m_io_space->read_dword(address, mem_mask);
}

WRITE32_MEMBER(cammu_device::mmu_w)
{
	u32 ssw = m_ssw_func();
	u32 address = offset << 2;

	// in supervisor mode, the first 8 pages are always mapped via the hard-wired tlb
	if ((ssw & 0x40000000) == 0 && (address & ~0x7fff) == 0)
	{
		switch (address & 0xf000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			// pages 0-3: main space pages 0-3
			m_main_space->write_dword(address, data, mem_mask);
			return;

		case 0x4000:
		case 0x5000:
			// pages 4-5: i/o space pages 0-1
			m_io_space->write_dword(address & 0x1fff, data, mem_mask);
			return;

		case 0x6000:
		case 0x7000:
			// pages 6-7: boot space pages 0-1
			m_boot_space->write_dword(address & 0x1fff, data, mem_mask);
			return;
		}
	}

	// FIXME: currently maps addresses with upper bits 0x00 or 0x7f1 to main memory and everything else to I/O
	if ((address & 0xff000000) == 0x00000000 || (address & 0xfff00000) == 0x7f100000)
		m_main_space->write_dword(address, data, mem_mask);
	else
		m_io_space->write_dword(address, data, mem_mask);
}
