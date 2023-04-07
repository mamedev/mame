// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Marvell MV64360/1/2

    System controller for PowerPC processors

***************************************************************************/

#include "emu.h"
#include "mv6436x.h"

//#define LOG_GENERAL    (1U << 0)
#define LOG_PCI_CONFIG (1U << 1)
#define LOG_PCI_MEM    (1U << 2)
#define LOG_PCI_IO     (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_PCI_CONFIG | LOG_PCI_MEM | LOG_PCI_IO)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MV64361, mv64361_device, "mv64361", "Marvell MV64361 System Controller")
DEFINE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device, "mv64361_pci_host", "Marvell MV64361 PCI Host")


//**************************************************************************
//  MV64361 SYSTEM CONTROLLER
//**************************************************************************

mv64361_device::mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MV64361, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void mv64361_device::device_start()
{
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	m_cpu_space->install_device(0xf1000000, 0xf100ffff, *static_cast<mv64361_device *>(this), &mv64361_device::register_map);
}

void mv64361_device::device_reset()
{
}

void mv64361_device::register_map(address_map &map)
{
	map(0x0000, 0x0007).rw(FUNC(mv64361_device::cpu_config_r), FUNC(mv64361_device::cpu_config_w));
}

uint32_t mv64361_device::cpu_config_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_GENERAL, "cpu_config_r\n");
	return 0x0000;
}

void mv64361_device::cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_GENERAL, "cpu_config_w: %08x\n", data);
}


//**************************************************************************
//  MV64361 PCI HOST
//**************************************************************************

mv64361_pci_host_device::mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)   :
	pci_host_device(mconfig, MV64361_PCI_HOST, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
	m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
{
	set_ids_host(0x11ab6460, 0x01, 0x00000000);
}

void mv64361_pci_host_device::device_start()
{
	pci_host_device::device_start();

	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	memory_space = &space(AS_PCI_MEM);
	io_space = &space(AS_PCI_IO);

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;

	m_cpu_space->install_device(0xf1000000, 0xf100ffff, *static_cast<mv64361_pci_host_device *>(this), &mv64361_pci_host_device::pci_map);

	m_cpu_space->install_read_handler (0xfe000000, 0xfe00ffff, read32s_delegate(*this, FUNC(mv64361_pci_host_device::pci_io_r)));
	m_cpu_space->install_write_handler(0xfe000000, 0xfe00ffff, write32s_delegate(*this, FUNC(mv64361_pci_host_device::pci_io_w)));
}

void mv64361_pci_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void mv64361_pci_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}

device_memory_interface::space_config_vector mv64361_pci_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void mv64361_pci_host_device::pci_map(address_map &map)
{
	// pci0 to pci1 offset is 0x80
	int o = m_busnum * 0x80;

	map(0x0cf8 - o, 0x0cfb - o).rw(FUNC(mv64361_pci_host_device::be_config_address_r), FUNC(mv64361_pci_host_device::be_config_address_w));
	map(0x0cfc - o, 0x0cff - o).rw(FUNC(mv64361_pci_host_device::be_config_data_r), FUNC(mv64361_pci_host_device::be_config_data_w));
}

uint32_t mv64361_pci_host_device::be_config_address_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = pci_host_device::config_address_r();
	LOGMASKED(LOG_PCI_CONFIG, "be_config_address_r: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	return swapendian_int32(data);
}

void mv64361_pci_host_device::be_config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_CONFIG, "be_config_address_w: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	pci_host_device::config_address_w(0, swapendian_int32(data), swapendian_int32(mem_mask));
}

uint32_t mv64361_pci_host_device::be_config_data_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = pci_host_device::config_data_r(0, swapendian_int32(mem_mask));
	LOGMASKED(LOG_PCI_CONFIG, "be_config_data_r: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	return swapendian_int32(data);
}

void mv64361_pci_host_device::be_config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_CONFIG, "be_config_data_w: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	pci_host_device::config_data_w(0, swapendian_int32(data), swapendian_int32(mem_mask));
}

uint32_t mv64361_pci_host_device::pci_io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = space(AS_PCI_IO).read_dword(offset * 4, swapendian_int32(mem_mask));
	LOGMASKED(LOG_PCI_IO, "pci_io_r[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	return swapendian_int32(data);
}

void mv64361_pci_host_device::pci_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_IO, "pci_io_w[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	space(AS_PCI_IO).write_dword(offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
}
