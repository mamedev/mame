// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc106.cpp - Motorola MPC106 PCI host bridge, aka "Grackle".

    Can support up to 4 PowerPC CPUs, up to 1 GiB of RAM, and 16 MiB of ROM

**********************************************************************/
#include "emu.h"
#include "mpc106.h"

DEFINE_DEVICE_TYPE(MPC106, mpc106_host_device, "mpc106", "Motorola MPC106 PCI Bridge/Memory Controller")

mpc106_host_device::mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mpc10x_host_device(mconfig, MPC106, tag, owner, clock)
{
	set_ids_host(0x10570002, 0x40, 0x000006);
}

void mpc106_host_device::config_map(address_map &map)
{
	mpc10x_host_device::config_map(map);
	map(0x72, 0x72).rw(FUNC(mpc106_host_device::pwrconfig2_r), FUNC(mpc106_host_device::pwrconfig2_w));
	map(0xa8, 0xab).rw(FUNC(mpc106_host_device::picr1_r), FUNC(mpc106_host_device::picr1_w));
}

void mpc106_host_device::device_start()
{
	mpc10x_host_device::device_start();
	revision = 0x40;
	m_pwrconfig2 = 0;
	save_item(NAME(m_pwrconfig2));
}

void mpc106_host_device::install_cpu_map(address_space &space)
{
	if (m_map_type == MAP_TYPE_A)
	{   // type A is PReP
		space.install_read_handler (0x80000000, 0x807fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		space.install_write_handler(0x80000000, 0x807fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));

		space.install_device(0x80000000, 0x80000cff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

		space.install_read_handler (0x81000000, 0xbf7fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x01000000>)));
		space.install_write_handler(0x81000000, 0xbf7fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x01000000>)));
		space.install_read_handler (0xc0000000, 0xfeffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		space.install_write_handler(0xc0000000, 0xfeffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
	}
	else if (m_map_type == MAP_TYPE_B)
	{   // type B is CHRP
		space.install_read_handler (0x80000000, 0xfcffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0x80000000>)));
		space.install_write_handler(0x80000000, 0xfcffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0x80000000>)));
		space.install_read_handler (0xfd000000, 0xfdffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		space.install_write_handler(0xfd000000, 0xfdffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
		space.install_read_handler (0xfe000000, 0xfe00ffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		space.install_write_handler(0xfe000000, 0xfe00ffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));
		space.install_read_handler (0xfe800000, 0xfebfffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x00800000>)));
		space.install_write_handler(0xfe800000, 0xfebfffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x00800000>)));

		install_config_access_map(space);
	}
}

// The CONFIG_ADDR/CONFIG_DATA window follows the host's byte order, so it has to be reinstalled
// when the firmware flips PICR1[LE_MODE] (MaciNTosh ARC firmware needs this).
void mpc106_host_device::install_config_access_map(address_space &space)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		space.install_device(0xfec00000, 0xfeefffff, *static_cast<mpc106_host_device *>(this), &mpc106_host_device::access_map_le);
	}
	else
	{
		space.install_device(0xfec00000, 0xfeefffff, *static_cast<mpc106_host_device *>(this), &mpc106_host_device::access_map_be);
	}
}

void mpc106_host_device::access_map_le(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(mpc106_host_device::config_address_r), FUNC(mpc106_host_device::config_address_w));
	map(0x00200000, 0x002fffff).rw(FUNC(mpc106_host_device::config_data_r), FUNC(mpc106_host_device::config_data_w));
}

void mpc106_host_device::access_map_be(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(mpc106_host_device::be_config_address_r), FUNC(mpc106_host_device::be_config_address_w));
	map(0x00200000, 0x002fffff).rw(FUNC(mpc106_host_device::be_config_data_r), FUNC(mpc106_host_device::be_config_data_w));
}

void mpc106_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									 u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	mpc10x_host_device::map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
			io_window_start, io_window_end, io_offset, io_space);

	// map RAM and ROM into the PCI memory space (because the PPC bus is 64 bits wide, and weirdness happens on DMA otherwise)
	memory_space->install_read_handler(0xff000000, 0xffffffff, read32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_r<0xff000000>)));

	memory_space->install_read_handler(0, m_ram_size - 1, read32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_r<0>)));
	memory_space->install_write_handler(0, m_ram_size - 1, write32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_w<0>)));
}

u8 mpc106_host_device::pwrconfig2_r()
{
		return m_pwrconfig2;
}

void mpc106_host_device::pwrconfig2_w(offs_t offset, u8 data)
{
	m_pwrconfig2 = data;
}

void mpc106_host_device::picr1_w(u32 data)
{
	const u32 changed = m_picr1 ^ data;
	m_picr1 = data;

	if ((changed & PICR1_LE_MODE) && (m_map_type == MAP_TYPE_B))
	{
		update_config_access();
	}
}
