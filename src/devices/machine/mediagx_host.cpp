// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    MediaGX host implementation (northbridge)

**************************************************************************************************/

#include "emu.h"
#include "mediagx_host.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(MEDIAGX_HOST, mediagx_host_device, "mediagx_host", "MediaGX X-Bus Host PCI")

mediagx_host_device::mediagx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, MEDIAGX_HOST, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

void mediagx_host_device::device_start()
{
	pci_host_device::device_start();

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	memory_space = &m_host_cpu->space(AS_PROGRAM);
	io_space = &m_host_cpu->space(AS_IO);

	m_ram.resize(m_ram_size/4);
}

void mediagx_host_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0007;
	status = 0x0280;

	m_pci_control[0] = 0x00;
	m_pci_control[1] = 0x96;

	m_pci_arbitration[0] = 0x80;
	m_pci_arbitration[1] = 0x00;

	remap_cb();
}

void mediagx_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x40, 0x41).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PCI Control Function %d read\n", offset + 1);
			return m_pci_control[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("PCI Control Function %d write %02x\n", offset + 1, data);
			m_pci_control[offset] = data;
		})
	);

	map(0x43, 0x44).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PCI Arbitration Control %d read\n", offset + 1);
			return m_pci_arbitration[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("PCI Arbitration Control %d write %02x\n", offset + 1, data);
			m_pci_arbitration[offset] = data;
		})
	);
}

void mediagx_host_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	memory_space->install_ram(0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);
//  memory_space->install_ram(0x000a0000, 0x000bffff, &m_ram[0x000a0000/4]);
	// temp
	memory_space->install_ram(0x000c0000, 0x000dffff, &m_ram[0x000c0000/4]);

	memory_space->install_ram          (0x00100000, 0x00efffff, &m_ram[0x00100000/4]);
	// memory hole at 15-16 mbytes
	//if(memory_hole_upper)
		memory_space->install_ram      (0x00f00000, 0x00ffffff, &m_ram[0x00f00000/4]);

	memory_space->install_ram          (0x01000000, m_ram_size-1, &m_ram[0x01000000/4]);
}
