// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    bandit.cpp - Apple "Bandit" and "Aspen" 60x bus/PCI bridges

**********************************************************************/
#include "emu.h"
#include "bandit.h"

#define VERBOSE (0)
#include "logmacro.h"

enum
{
	AS_PCI_MEM = 1,
	AS_PCI_IO = 2
};

DEFINE_DEVICE_TYPE(BANDIT, bandit_host_device, "banditpci", "Apple Bandit PowerPC-to-PCI bridge")
DEFINE_DEVICE_TYPE(ASPEN, aspen_host_device, "aspenpci", "Apple Aspen PowerPC-to-PCI bridge and memory controller")

void bandit_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
}

bandit_host_device::bandit_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pci_host_device(mconfig, type, tag, owner, clock)
	, m_last_config_address(0xffffffff)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_dev_offset(0)
{
	set_ids_host(0x106b0001, 0x00, 0x00000000);
}

bandit_host_device::bandit_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
   : bandit_host_device(mconfig, BANDIT, tag, owner, clock)
{

}

aspen_host_device::aspen_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: bandit_host_device(mconfig, ASPEN, tag, owner, clock)
{
}

void bandit_host_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PCI_CONFIG);
	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;
	command = 0x0006;
	status = 0x0080;
	revision = 0;

	m_cpu_space->install_read_handler(0x80000000, 0xefffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0x80000000>)));
	m_cpu_space->install_write_handler(0x80000000, 0xefffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0x80000000>)));

	// TODO: PCI I/O space is at Fn000000-Fn7FFFFF, but it's unclear where in the PCI space that maps to

	switch (m_dev_offset)
	{
		case 0:
			m_cpu_space->install_read_handler(0xf1000000, 0xf1ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0xf1000000>)));
			m_cpu_space->install_write_handler(0xf1000000, 0xf1ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0xf1000000>)));
			break;

		case 1:
			m_cpu_space->install_read_handler(0xf3000000, 0xf3ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0xf3000000>)));
			m_cpu_space->install_write_handler(0xf3000000, 0xf3ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0xf3000000>)));
			break;

		case 2:
			m_cpu_space->install_read_handler(0xf5000000, 0xf5ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0xf5000000>)));
			m_cpu_space->install_write_handler(0xf5000000, 0xf5ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0xf5000000>)));
			break;

		case 3:
			m_cpu_space->install_read_handler(0xf7000000, 0xf7ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0xf7000000>)));
			m_cpu_space->install_write_handler(0xf7000000, 0xf7ffffff, emu::rw_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0xf7000000>)));
			break;
	}

	const u32 base = 0xf0000000 + (0x02000000 * m_dev_offset);
	m_cpu_space->install_device(base, base + 0x01ffffff, *static_cast<bandit_host_device *>(this), &bandit_host_device::cpu_map);
}

device_memory_interface::space_config_vector bandit_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void bandit_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void bandit_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void bandit_host_device::cpu_map(address_map &map)
{
	map(0x00800000, 0x00bfffff).rw(FUNC(bandit_host_device::be_config_address_r), FUNC(bandit_host_device::be_config_address_w));
	map(0x00c00000, 0x00ffffff).rw(FUNC(bandit_host_device::be_config_data_r), FUNC(bandit_host_device::be_config_data_w));
}

u32 bandit_host_device::be_config_address_r()
{
	return m_last_config_address;
}

void bandit_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 tempdata = swapendian_int32(data);

	m_last_config_address = tempdata;

	if ((tempdata & 3) == 1)
	{
			tempdata |= 0x80000000;
	}

	pci_host_device::config_address_w(offset, tempdata, mem_mask);
}

u32 bandit_host_device::be_config_data_r(offs_t offset, u32 mem_mask)
{
	return swapendian_int32(pci_host_device::config_data_ex_r(offset, mem_mask));
}

void bandit_host_device::be_config_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	// printf("config_data_w: %08x @ %08x mask %08x\n", data, offset, mem_mask);

	pci_host_device::config_data_ex_w(offset, swapendian_int32(data), mem_mask);
}

template <u32 Base>
u32 bandit_host_device::pci_memory_r(offs_t offset, u32 mem_mask)
{
	return swapendian_int32(this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
}

template <u32 Base>
void bandit_host_device::pci_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32((mem_mask)));
}

template u32 bandit_host_device::pci_memory_r<0x80000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0x80000000>(offs_t offset, u32 data, u32 mem_mask);
template u32 bandit_host_device::pci_memory_r<0xf1000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0xf1000000>(offs_t offset, u32 data, u32 mem_mask);
template u32 bandit_host_device::pci_memory_r<0xf3000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0xf3000000>(offs_t offset, u32 data, u32 mem_mask);
template u32 bandit_host_device::pci_memory_r<0xf5000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0xf5000000>(offs_t offset, u32 data, u32 mem_mask);
template u32 bandit_host_device::pci_memory_r<0xf7000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0xf7000000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 bandit_host_device::pci_io_r(offs_t offset, u32 mem_mask)
{
	return swapendian_int32(this->space(AS_PCI_IO).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
}

template <u32 Base>
void bandit_host_device::pci_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_IO).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32((mem_mask)));
}

// map PCI memory and I/O space stuff here
void bandit_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									 u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
}

u32 aspen_host_device::be_config_address_r()
{
	return m_last_config_address;
}

void aspen_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_last_config_address = data;

	if ((data & 3) == 1)
	{
		data |= 0x80000000;
	}

	pci_host_device::config_address_w(offset, data, mem_mask);
}
