// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C505 ISA/VL PCI bridge

    TODO:
    - no PCI devices behind the bridge yet, so the PCI memory and I/O
      spaces are the bridge's own and not connected to the host CPU

*/

#include "emu.h"
#include "vt82c505.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(VT82C505, vt82c505_device, "vt82c505", "VIA VT82C505 PCI bridge")


vt82c505_device::vt82c505_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, VT82C505, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_pci_mem_config("pci_mem", ENDIANNESS_LITTLE, 32, 32)
	, m_pci_io_config("pci_io", ENDIANNESS_LITTLE, 32, 16)
{
	set_ids_host(0x11060505, 0x00, 0x00000000);
}

device_memory_interface::space_config_vector vt82c505_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_pci_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_pci_io_config));
	return r;
}

void vt82c505_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x80, 0x83).r(FUNC(vt82c505_device::config_80_r)); // DIP switch / revision, memory size, buffer control, VLB interface timing
	map(0x84, 0x87).w(FUNC(vt82c505_device::config_84_w));
	map(0x88, 0x8b).w(FUNC(vt82c505_device::config_88_w));
}

void vt82c505_device::device_start()
{
	pci_host_device::device_start();

	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));

	m_host_cpu->space(AS_IO).install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	command = 0x0007;
	command_mask = 0x0000;

	std::fill(std::begin(m_window_addr), std::end(m_window_addr), 0);
	std::fill(std::begin(m_window_attr), std::end(m_window_attr), 0);

	save_item(NAME(m_window_addr));
	save_item(NAME(m_window_attr));
}

u32 vt82c505_device::config_80_r()
{
	return 0x00000100;
}

void vt82c505_device::config_84_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)  // RX87: memory window 1 A31:24
	{
		m_window_addr[0] = (m_window_addr[0] & 0xff00) | (data & 0x000000ff);
		logerror("memory window #1 A31:24 set: %04x\n", m_window_addr[0]);
	}
}

void vt82c505_device::config_88_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_24_31)  // RX88: memory window 1 A23:16
	{
		m_window_addr[0] = (m_window_addr[0] & 0x00ff) | ((data & 0xff000000) >> 24);
		logerror("memory window #1 A23:16 set: %04x\n", m_window_addr[0]);
	}
	if (ACCESSING_BITS_16_23)  // RX89: memory window 1 attributes
	{
		m_window_attr[0] = (data & 0x00ff0000) >> 16;
		logerror("memory window #1 attributes set: %02x\n", m_window_attr[0]);
	}
}
