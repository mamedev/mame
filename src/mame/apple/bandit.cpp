// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    bandit.cpp - Apple "Bandit" 60x bus/PCI bridge

**********************************************************************/
#include "emu.h"
#include "bandit.h"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (0)
#include "logmacro.h"

enum
{
	AS_PCI_MEM = 1,
	AS_PCI_IO = 2
};

DEFINE_DEVICE_TYPE(BANDIT, bandit_host_device, "banditpci", "Apple Bandit PowerPC-to-PCI Bridge")

void bandit_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
}

bandit_host_device::bandit_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_host_device(mconfig, BANDIT, tag, owner, clock)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_dev_offset(0)
{
	set_ids_host(0x106b0001, 0x00, 0x00000000);
}

void bandit_host_device::device_start()
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
	revision = 0;

	// don't know the actual mappings yet, just guess
	m_cpu_space->install_read_handler(0xf3000000, 0xf7ffffff, read32s_delegate(*this, FUNC(bandit_host_device::pci_memory_r<0xf3000000>)));
	m_cpu_space->install_write_handler(0xf3000000, 0xf7ffffff, write32s_delegate(*this, FUNC(bandit_host_device::pci_memory_w<0xf3000000>)));
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

void bandit_host_device::map(address_map &map)
{
	map(0x00800000, 0x00bfffff).rw(FUNC(bandit_host_device::be_config_address_r), FUNC(bandit_host_device::be_config_address_w));
	map(0x00c00000, 0x00ffffff).rw(FUNC(bandit_host_device::be_config_data_r), FUNC(bandit_host_device::be_config_data_w));
}

u32 bandit_host_device::be_config_address_r()
{
	u32 temp = pci_host_device::config_address_r();
	return (temp >> 24) | (temp << 24) | ((temp & 0xff00) << 8) | ((temp & 0xff0000) >> 8);
}

void bandit_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	pci_host_device::config_address_w(offset, data|0x80000000, mem_mask);
}

u32 bandit_host_device::be_config_data_r(offs_t offset, u32 mem_mask)
{
	return pci_host_device::config_data_r(offset, mem_mask);
}

void bandit_host_device::be_config_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 tempdata;

	// printf("config_data_w: %08x @ %08x mask %08x\n", data, offset, mem_mask);

	tempdata = (data >> 24) | (data << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8);
	pci_host_device::config_data_w(offset, tempdata, mem_mask);
}

template <u32 Base>
u32 bandit_host_device::pci_memory_r(offs_t offset, u32 mem_mask)
{
	u32 result = this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), mem_mask);
	return result;
}

template <u32 Base>
void bandit_host_device::pci_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), data, mem_mask);
}

template u32 bandit_host_device::pci_memory_r<0xf3000000>(offs_t offset, u32 mem_mask);
template void bandit_host_device::pci_memory_w<0xf3000000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 bandit_host_device::pci_io_r(offs_t offset, u32 mem_mask)
{
	u32 result = this->space(AS_PCI_IO).read_dword(Base + (offset * 4), mem_mask);
	return result;
}

template <u32 Base>
void bandit_host_device::pci_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_PCI_IO).write_dword(Base + (offset * 4), data, mem_mask);
}

// map PCI memory and I/O space stuff here
void bandit_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									 u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
}
