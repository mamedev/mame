// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc105.cpp - Motorola MPC105 PCI host bridge, aka "Eagle".
    Emulation by R. Belmont

    TODO: cache/DRAM timing, error reporting, external configuration
    ports, interrupt acknowledge, 32-bit processor bus, flash writes.

**********************************************************************/

#include "emu.h"
#include "mpc105.h"

#include <algorithm>
#include <iterator>

DEFINE_DEVICE_TYPE(MPC105, mpc105_host_device, "mpc105", "Motorola MPC105 PCI Bridge/Memory Controller")

mpc105_host_device::mpc105_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mpc10x_host_device(mconfig, MPC105, tag, owner, clock)
	, m_picr2(0)
	, m_mccr{ 0, 0, 0, 0 }
{
	set_ids_host(0x1057'0001, 0x00, 0);
}

void mpc105_host_device::config_map(address_map &map)
{
	mpc10x_host_device::config_map(map);
	// This host has a type 0 header, without the PCI-to-PCI bridge windows.
	map(0x10, 0x3b).unmaprw();
	map(0x3c, 0x3f).lr8(NAME([] () -> u8 { return 0; })).nopw();
	map(0x40, 0x42).lr8(NAME([] () -> u8 { return 0; }));
	map(0x44, 0x45).lr16(NAME([] () -> u16 { return 0; }));
	map(0xa8, 0xab).rw(FUNC(mpc105_host_device::picr1_r), FUNC(mpc105_host_device::picr1_w));
	map(0xac, 0xaf).rw(FUNC(mpc105_host_device::picr2_r), FUNC(mpc105_host_device::picr2_w));
	map(0xf0, 0xff).rw(FUNC(mpc105_host_device::mccr_r), FUNC(mpc105_host_device::mccr_w));
}

void mpc105_host_device::device_start()
{
	mpc10x_host_device::device_start();
	save_item(NAME(m_picr2));
	save_item(NAME(m_mccr));
}

u8 mpc105_host_device::header_type_r()
{
	return 0;
}

void mpc105_host_device::device_reset()
{
	mpc10x_host_device::device_reset();
	command = 0x0006;
	status = 0x0080;
	m_pwrconfig1 = 0;
	std::fill(std::begin(m_memory_starts), std::end(m_memory_starts), 0);
	std::fill(std::begin(m_memory_ends), std::end(m_memory_ends), 0);
	m_memory_bank_enable = 0;
	// RCS0: ROM on the processor bus; XATS: the map strap is read-only.
	m_picr1 = 0xff10'0010 | ((m_map_type == MAP_TYPE_A) ? 0x0001'0000 : 0);
	m_picr2 = 0x000c'060c;
	m_mccr[0] = 0xff02'0000;
	m_mccr[1] = 0x0000'0003;
	m_mccr[2] = 0;
	m_mccr[3] = 0x0010'0000;
	install_ram();
	update_config_access();
}

void mpc105_host_device::install_cpu_map(address_space &space)
{
	if (m_map_type == MAP_TYPE_A)
	{
		space.install_read_handler (0x8100'0000, 0xbf7f'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_io_r<0x0100'0000>)));
		space.install_write_handler(0x8100'0000, 0xbf7f'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_io_w<0x0100'0000>)));
		space.install_read_handler (0xc000'0000, 0xfeff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_r<0>)));
		space.install_write_handler(0xc000'0000, 0xfeff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_w<0>)));
	}
	else
	{
		// MPC105 Map B preserves addresses on PCI, including the I/O windows.
		space.install_read_handler (0x8000'0000, 0xefff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_r<0x8000'0000>)));
		space.install_write_handler(0x8000'0000, 0xefff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_w<0x8000'0000>)));
		space.install_read_handler (0xf000'0000, 0xf07f'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_io_r<0xf000'0000>)));
		space.install_write_handler(0xf000'0000, 0xf07f'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_io_w<0xf000'0000>)));
		space.install_read_handler (0xf100'0000, 0xf1ff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_r<0xf100'0000>)));
		space.install_write_handler(0xf100'0000, 0xf1ff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_w<0xf100'0000>)));
		space.install_read_handler (0xf800'0000, 0xf8ff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_io_r<0xf800'0000>)));
		space.install_write_handler(0xf800'0000, 0xf8ff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_io_w<0xf800'0000>)));
		space.install_read_handler (0xf900'0000, 0xfeff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_r<0xf900'0000>)));
		space.install_write_handler(0xf900'0000, 0xfeff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_memory_w<0xf900'0000>)));
	}
	install_config_access_map(space);
}

void mpc105_host_device::install_config_access_map(address_space &space)
{
	if (m_map_type == MAP_TYPE_A)
	{
		// Replace the previous configuration window when XIO_MODE changes.
		space.install_read_handler (0x8000'0000, 0x807f'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::pci_io_a_r)));
		space.install_write_handler(0x8000'0000, 0x807f'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::pci_io_a_w)));
		const u32 address = (m_picr1 & PICR1_XIO_MODE) ? 0x8006'7018 : 0x8000'0cf8;
		space.install_device(address, address + 7, *this, &mpc105_host_device::access_map_a);
	}
	else
	{
		space.install_device(0xf080'0000, 0xf0df'ffff, *this, &mpc105_host_device::access_map_b);
		space.install_read_handler (0xf8ff'f000, 0xf8ff'f0ff, read32_delegate(*this, FUNC(mpc105_host_device::internal_config_r)));
		space.install_write_handler(0xf8ff'f000, 0xf8ff'f0ff, write32s_delegate(*this, FUNC(mpc105_host_device::internal_config_w)));
	}
}

void mpc105_host_device::access_map_a(address_map &map)
{
	// a little-endian CPU's address munging swaps the two registers on the bus
	if (m_picr1 & PICR1_LE_MODE)
	{
		map(0, 3).rw(FUNC(mpc105_host_device::config_data_mp_r), FUNC(mpc105_host_device::config_data_w));
		map(4, 7).rw(FUNC(mpc105_host_device::config_address_a_r), FUNC(mpc105_host_device::config_address_a_w));
	}
	else
	{
		map(0, 3).rw(FUNC(mpc105_host_device::config_address_a_r), FUNC(mpc105_host_device::config_address_a_w));
		map(4, 7).rw(FUNC(mpc105_host_device::be_config_data_mp_r), FUNC(mpc105_host_device::be_config_data_w));
	}
}

// Map A uses PCI configuration mechanism 1: only full DWORD accesses select
// CONFIG_ADDR. Byte and word accesses at CF8-CFB remain ordinary PCI I/O.
u32 mpc105_host_device::config_address_a_r(offs_t offset, u32 mem_mask)
{
	if (mem_mask != 0xffff'ffff)
	{
		// pci_io_r undoes the little-endian munge itself
		return pci_io_r<0>((m_picr1 & PICR1_LE_MODE) ? (0xcfc / 4) : (0xcf8 / 4), mem_mask);
	}
	return (m_picr1 & PICR1_LE_MODE) ? config_address_r() : be_config_address_r();
}

void mpc105_host_device::config_address_a_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask != 0xffff'ffff)
	{
		pci_io_w<0>((m_picr1 & PICR1_LE_MODE) ? (0xcfc / 4) : (0xcf8 / 4), data, mem_mask);
	}
	else if (m_picr1 & PICR1_LE_MODE)
	{
		config_address_w(0, data, mem_mask);
	}
	else
	{
		be_config_address_w(0, data, mem_mask);
	}
}

void mpc105_host_device::access_map_b(address_map &map)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		map(0x00'0000, 0x3f'ffff).rw(FUNC(mpc105_host_device::config_address_r), FUNC(mpc105_host_device::config_address_w));
		map(0x40'0000, 0x5f'ffff).rw(FUNC(mpc105_host_device::config_data_mp_r), FUNC(mpc105_host_device::config_data_w));
	}
	else
	{
		map(0x00'0000, 0x3f'ffff).rw(FUNC(mpc105_host_device::be_config_address_r), FUNC(mpc105_host_device::be_config_address_w));
		map(0x40'0000, 0x5f'ffff).rw(FUNC(mpc105_host_device::be_config_data_mp_r), FUNC(mpc105_host_device::be_config_data_w));
	}
}

// CF_MP_ID describes the processor making this transaction, including debugger reads.
// The configuration latch and all the other host registers are shared by both CPUs.
u32 mpc105_host_device::config_data_mp_r(address_space &space, offs_t offset, u32 mem_mask)
{
	u32 data = config_data_r(offset, mem_mask);
	if (((config_address & 0x80ff'fffc) == 0x8000'00a8) && (&space != m_cpu_space))
	{
		data |= 0x0000'8000;
	}
	return data;
}

u32 mpc105_host_device::be_config_data_mp_r(address_space &space, offs_t offset, u32 mem_mask)
{
	return swapendian_int32(config_data_mp_r(space, offset, swapendian_int32(mem_mask)));
}

u32 mpc105_host_device::internal_config_r(address_space &cpu_space, offs_t offset, u32 mem_mask)
{
	// Access our configuration space directly, independent of CONFIG_ADDR.
	const bool little = bool(m_picr1 & PICR1_LE_MODE);
	const offs_t reg = little ? (offset ^ 1) : offset;
	u32 data = space(AS_PCI_CONFIG).read_dword(reg * 4, little ? mem_mask : swapendian_int32(mem_mask));
	if ((reg == (0xa8 / 4)) && (&cpu_space != m_cpu_space))
	{
		data |= 0x0000'8000;
	}
	return little ? data : swapendian_int32(data);
}

void mpc105_host_device::internal_config_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		space(AS_PCI_CONFIG).write_dword((offset ^ 1) * 4, data, mem_mask);
	}
	else
	{
		space(AS_PCI_CONFIG).write_dword(offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	}
}

u32 mpc105_host_device::pci_io_a_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_XIO_MODE)
	{
		offset = ((offset & 0x1f'fc00) >> 7) | (offset & 7);
	}
	else if (offset >= 0x4000)
	{
		return 0xffff'ffff;
	}
	return pci_io_r<0>(offset, mem_mask);
}

void mpc105_host_device::pci_io_a_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_XIO_MODE)
	{
		offset = ((offset & 0x1f'fc00) >> 7) | (offset & 7);
	}
	else if (offset >= 0x4000)
	{
		return;
	}
	pci_io_w<0>(offset, data, mem_mask);
}

void mpc105_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
		u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	mpc10x_host_device::map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
			io_window_start, io_window_end, io_offset, io_space);

	const u32 base = (m_map_type == MAP_TYPE_A) ? 0x8000'0000 : 0;
	memory_space->install_read_handler (base, base + 0x3fff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::cpu_memory_r<0>)));
	memory_space->install_write_handler(base, base + 0x3fff'ffff, write32s_delegate(*this, FUNC(mpc105_host_device::cpu_memory_w<0>)));
	if (m_map_type == MAP_TYPE_B)
	{
		memory_space->install_read_handler(0xff00'0000, 0xffff'ffff, read32s_delegate(*this, FUNC(mpc105_host_device::cpu_memory_r<0xff00'0000>)));
	}
}

void mpc105_host_device::install_ram()
{
	for (address_space *space : m_cpu_spaces)
	{
		space->unmap_readwrite(0, 0x3fff'ffff);
	}
	if (BIT(m_mccr[0], 19)) // MEMGO enables the RAM interface.
	{
		install_memory_banks(false, false);
	}
}

void mpc105_host_device::memory_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= 2)
	{
		mem_mask &= 0x0303'0303;
	}
	mpc10x_host_device::memory_start_w(offset, data, mem_mask);
	install_ram();
}

void mpc105_host_device::memory_end_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= 2)
	{
		mem_mask &= 0x0303'0303;
	}
	mpc10x_host_device::memory_end_w(offset, data, mem_mask);
	install_ram();
}

void mpc105_host_device::picr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 previous = m_picr1;
	// RCS0, XATS and CF_MP_ID are read-only; bits 14 and 8 are reserved.
	mem_mask &= ~0x0011'c100;
	COMBINE_DATA(&m_picr1);
	if ((previous ^ m_picr1) & (PICR1_LE_MODE | PICR1_XIO_MODE))
	{
		update_config_access();
	}
}

u32 mpc105_host_device::picr2_r()
{
	return m_picr2;
}

void mpc105_host_device::picr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	mem_mask &= ~0x2c00'8800;
	COMBINE_DATA(&m_picr2);
}

u32 mpc105_host_device::mccr_r(offs_t offset)
{
	return m_mccr[offset];
}

void mpc105_host_device::mccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	// MCCR1[FNR,32N64] are straps, and MCCR2/3/4 have reserved bits.
	static constexpr u32 WRITABLE[4] = { 0xff9f'ffff, 0xfffc'ffff, 0xfff7'ffff, 0xff3f'ffff };
	mem_mask &= WRITABLE[offset];
	COMBINE_DATA(&m_mccr[offset]);
	if (offset == 0)
	{
		install_ram();
	}
}
