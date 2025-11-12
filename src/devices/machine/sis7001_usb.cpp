// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SiS 7001 USB Host controller (OHCI)

TODO:
- Stub interface, to be merged with pci-usb;
- PCI values omitted from docs, assumes same as OpenHCI;

**************************************************************************************************/

#include "emu.h"
#include "sis7001_usb.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SIS7001_USB, sis7001_usb_device, "sis7001_usb", "SiS 7001 USB Host Controller")

sis7001_usb_device::sis7001_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS7001_USB, tag, owner, clock)
{

}

void sis7001_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// no vendor regs?
}

void sis7001_usb_device::map(address_map &map)
{
	// HcRevision: operational mode
	// 1 ---- ---- Legacy PS/2 present
	// - 0001 0000 OpenHCI v1.0
	map(0x000, 0x003).lr32(NAME([]() { return 0x00000110; }));
	// ...
	// HcFmInterval (Windows OSes fails if this isn't r/w)
	map(0x034, 0x037).lrw32(
		NAME([this]() { return m_HcFmInterval; } ),
		NAME([this](offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_HcFmInterval);
			LOG("Write HcFmInterval %08x & %08x\n", data, mem_mask);
		})
	);
	// ...
	// HcRhDescriptorA
	// 0000 0001 ---- ---- ---- ---- ---- ---- POTPGT 2 ms
	// ---- ---- xxxx xxxx xxxx x-xx ---- ---- (writeable bits by host controller driver)
	// ---- ---- ---- ---- ---- -0-- ---- ---- DT DeviceType (0) not a compound device
	// ---- ---- ---- ---- ---- ---- xxxx xxxx NDP NumberDownstreamPorts (15 max)
	map(0x048, 0x04b).lr32(NAME([this]() { return 0x01000000 | m_downstream_ports; }));
	// ...
//  map(0x05c, 0x05f) HcRhPortStatus[3] last item for function 2 (3 ports), missing on function 3 (2)

	// legacy support mode (8-bit each)
//  map(0x100, 0x100) [Hce]Control, bit 0 enables emulation mode
//  map(0x104, 0x104) [Hce]Input
//  map(0x108, 0x108) [Hce]Output
//  map(0x10c, 0x10f) [Hce]Status
}

void sis7001_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: overrides I/O ports $0060-$0064 (emulation mode)
}

void sis7001_usb_device::device_start()
{
	pci_device::device_start();

	// Docs don't mention it, should be 4KB
	add_map(4096, M_MEM, FUNC(sis7001_usb_device::map));

	// INTD#
	intr_pin = 4;
}


void sis7001_usb_device::device_reset()
{
	pci_device::device_reset();

	// TODO: unverified
	command = 0x0000;
	status = 0x0000;
	m_HcFmInterval = 0x2edf;
}
