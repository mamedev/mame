// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*

    Cyrix Cx5510 MediaGX companion chip

    TODO:
    - only the configuration registers are stored, nothing is decoded
    - no PCI devices are mapped through it, so the PCI memory and I/O
      spaces are its own and not connected to the host CPU

*/

#include "emu.h"
#include "cx5510.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(CX5510, cx5510_device, "cx5510", "Cyrix Cx5510")


cx5510_device::cx5510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, CX5510, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_pci_mem_config("pci_mem", ENDIANNESS_LITTLE, 32, 32)
	, m_pci_io_config("pci_io", ENDIANNESS_LITTLE, 32, 16)
{
	set_ids(0x10780000, 0x00, 0x060100, 0x00000000);
}

device_memory_interface::space_config_vector cx5510_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_pci_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_pci_io_config));
	return r;
}

void cx5510_device::device_start()
{
	pci_host_device::device_start();

	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	m_host_cpu->space(AS_IO).install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	save_item(NAME(m_regs));
}

void cx5510_device::config_map(address_map &map)
{
	// plain register file, the header is not decoded either
	map(0x00, 0x03).lr32(NAME([this] () -> uint32_t { return (main_id >> 16) | (main_id << 16); }));
	map(0x04, 0xff).lrw32(
			NAME([this] (offs_t offset) -> uint32_t { return m_regs[1 + offset]; }),
			NAME([this] (offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_regs[1 + offset]); }));
}
