// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc106.cpp - Motorola MPC106 PCI host bridge, aka "Grackle".

    Can support up to 4 PowerPC CPUs, up to 1 GiB of RAM, and 16 MiB of ROM

**********************************************************************/
#include "emu.h"
#include "mpc106.h"

enum
{
	AS_PCI_MEM = 1,
	AS_PCI_IO = 2
};

DEFINE_DEVICE_TYPE(MPC106, mpc106_host_device, "mpc106", "Motorola MPC106 PCI Bridge/Memory Controller")

void mpc106_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x70, 0x71).rw(FUNC(mpc106_host_device::pwrconfig1_r), FUNC(mpc106_host_device::pwrconfig1_w));
	map(0x72, 0x72).rw(FUNC(mpc106_host_device::pwrconfig2_r), FUNC(mpc106_host_device::pwrconfig2_w));
}

mpc106_host_device::mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_host_device(mconfig, MPC106, tag, owner, clock)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
	set_ids_host(0x10570002, 0x40, 0x000006);
}

void mpc106_host_device::set_ram_size(int _ram_size)
{
	m_ram_size = _ram_size;
}

void mpc106_host_device::set_rom_tag(const char *tag)
{
	m_rom_tag = tag;
}

void mpc106_host_device::set_map_type(map_type maptype)
{
	m_map_type = maptype;
}

void mpc106_host_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PCI_CONFIG);
	memory_space = &space(AS_PCI_MEM);
	io_space = &space(AS_PCI_IO);

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;
	command = 0x0006;
	status = 0x0080;

	m_rom = device().machine().root_device().memregion(m_rom_tag)->base();
	m_rom_size = device().machine().root_device().memregion(m_rom_tag)->bytes();

	m_ram.resize(m_ram_size/4);

	m_pwrconfig1 = m_pwrconfig2 = 0;
	m_last_config_address = -1;

	u64 rom_base = 0x100000000ULL - m_rom_size;
	m_cpu_space->install_rom(rom_base, 0xffffffff, m_rom);

	if (m_map_type == MAP_TYPE_A)
	{   // type A is PReP
		m_cpu_space->install_read_handler (0x80000000, 0x807fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		m_cpu_space->install_write_handler(0x80000000, 0x807fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));

		m_cpu_space->install_device(0x80000000, 0x80000cff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

		m_cpu_space->install_read_handler (0x81000000, 0xbf7fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x01000000>)));
		m_cpu_space->install_write_handler(0x81000000, 0xbf7fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x01000000>)));
		m_cpu_space->install_read_handler (0xc0000000, 0xfeffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		m_cpu_space->install_write_handler(0xc0000000, 0xfeffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
	}
	else if (m_map_type == MAP_TYPE_B)
	{   // type B is CHRP
		m_cpu_space->install_read_handler (0x80000000, 0xfcffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0x80000000>)));
		m_cpu_space->install_write_handler(0x80000000, 0xfcffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0x80000000>)));
		m_cpu_space->install_read_handler (0xfd000000, 0xfdffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		m_cpu_space->install_write_handler(0xfd000000, 0xfdffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
		m_cpu_space->install_read_handler (0xfe000000, 0xfe00ffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		m_cpu_space->install_write_handler(0xfe000000, 0xfe00ffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));
		m_cpu_space->install_read_handler (0xfe800000, 0xfebfffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x00800000>)));
		m_cpu_space->install_write_handler(0xfe800000, 0xfebfffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x00800000>)));

		m_cpu_space->install_device(0xfec00000, 0xfeefffff, *static_cast<mpc106_host_device *>(this), &mpc106_host_device::access_map);
	}
}

device_memory_interface::space_config_vector mpc106_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void mpc106_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void mpc106_host_device::device_reset()
{
	pci_host_device::device_reset();
	m_last_config_address = -1;
}

void mpc106_host_device::access_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(mpc106_host_device::be_config_address_r), FUNC(mpc106_host_device::be_config_address_w));
	map(0x00200000, 0x002fffff).rw(FUNC(mpc106_host_device::be_config_data_r), FUNC(mpc106_host_device::be_config_data_w));
}

u32 mpc106_host_device::be_config_address_r()
{
	u32 temp = pci_host_device::config_address_r();
	return (temp>>24) | (temp<<24) | ((temp & 0xff00) << 8) | ((temp & 0xff0000) >> 8);
}

void mpc106_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 tempdata;

	//printf("config_address_w: %08x mask %08x\n", data, mem_mask);

	tempdata = (data >> 24) | (data << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8);
	pci_host_device::config_address_w(offset, tempdata, mem_mask);
	m_last_config_address = tempdata & 0xffffff00;
}

u32 mpc106_host_device::be_config_data_r(offs_t offset, u32 mem_mask)
{
	// config registers inside the MPC106 itself are little-endian and must be flipped if the host CPU is BE.
	// TODO: we just assume a BE host CPU for now.
	if (m_last_config_address == 0x80000000)
	{
		return pci_host_device::config_data_r(offset, mem_mask);
	}
	else
	{
		u32 temp = pci_host_device::config_data_r(offset, mem_mask);
		return (temp >> 24) | (temp << 24) | ((temp & 0xff00) << 8) | ((temp & 0xff0000) >> 8);
	}
}

void mpc106_host_device::be_config_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_last_config_address == 0x80000000)
	{
		pci_host_device::config_data_w(offset, data, mem_mask);
	}
	else
	{
		u32 tempdata;

		//printf("config_data_w: %08x mask %08x\n", data, mem_mask);

		tempdata = (data >> 24) | (data << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8);
		pci_host_device::config_data_w(offset, tempdata, mem_mask);
	}
}

template <u32 Base>
u32 mpc106_host_device::pci_memory_r(offs_t offset, u32 mem_mask)
{
	u32 result = this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), mem_mask);
	return result;
}

template <u32 Base>
void mpc106_host_device::pci_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), data, mem_mask);
}

template u32 mpc106_host_device::pci_memory_r<0>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_memory_r<0x80000000>(offs_t offset, u32 mem_mask);

template void mpc106_host_device::pci_memory_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_memory_w<0x80000000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 mpc106_host_device::pci_io_r(offs_t offset, u32 mem_mask)
{
	u32 result = this->space(AS_PCI_IO).read_dword(Base + (offset * 4), mem_mask);
	return result;
}

template <u32 Base>
void mpc106_host_device::pci_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_IO).write_dword(Base + (offset * 4), data, mem_mask);
}

template u32 mpc106_host_device::pci_io_r<0>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_io_r<0x01000000>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_io_r<0x00800000>(offs_t offset, u32 mem_mask);

template void mpc106_host_device::pci_io_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_io_w<0x01000000>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_io_w<0x00800000>(offs_t offset, u32 data, u32 mem_mask);

// map PCI memory and I/O space stuff here
void mpc106_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									 u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	u64 rom_base = 0x100000000ULL - m_rom_size;
	memory_space->install_rom(rom_base, 0xffffffff, m_rom);
}

u16 mpc106_host_device::pwrconfig1_r()
{
	return m_pwrconfig1;
}

void mpc106_host_device::pwrconfig1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pwrconfig1);
}

u8 mpc106_host_device::pwrconfig2_r()
{
		return m_pwrconfig2;
}

void mpc106_host_device::pwrconfig2_w(offs_t offset, u8 data)
{
	m_pwrconfig2 = data;
}
