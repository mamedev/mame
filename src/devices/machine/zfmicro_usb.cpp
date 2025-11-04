// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

ZFMicro USB Host controller (OHCI)

TODO:
- Stub interface, to be merged with pci-usb;
- PCI values omitted from docs, assumes same as OpenHCI;

**************************************************************************************************/

#include "emu.h"
#include "zfmicro_usb.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(ZFMICRO_USB, zfmicro_usb_device, "zfmicro_usb", "ZFMicro PCIUSB Host Controller")

zfmicro_usb_device::zfmicro_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, ZFMICRO_USB, tag, owner, clock)
{
	// Compaq vendor
	set_ids(0x0e11a0f8, 0x00, 0x0c0310, 0x00);
	// TODO: should really read from a std::list interface
	m_downstream_ports = 2;
}

void zfmicro_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//  map(0x40, 0x43) ASIC Test Mode Enable
//  map(0x44, 0x44) ASIC Operational Mode Enable
//  ---- ---x Data Buffer Region 16
//  ---- ---1 16 bytes data buffer
//  ---- ---0 32 bytes
}

void zfmicro_usb_device::map(address_map &map)
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
	// 0000 0001 ---- ---- ---- ---- ---- ---- POTPGT (unknown for this, assume 2 ms)
	// ---- ---- xxxx xxxx xxxx x-xx ---- ---- (writeable bits by host controller driver)
	// ---- ---- ---- ---- ---- -0-- ---- ---- DT DeviceType (0) not a compound device
	// ---- ---- ---- ---- ---- ---- xxxx xxxx NDP NumberDownstreamPorts (15 max)
	map(0x048, 0x04b).lr32(NAME([this]() { return 0x01000000 | m_downstream_ports; }));
	// ...

	// legacy support mode (8-bit each)
//  map(0x100, 0x100) HceControl, bit 0 enables emulation mode
//  map(0x104, 0x104) HceInput
//  map(0x108, 0x108) HceOutput
//  map(0x10c, 0x10f) HceStatus
}

void zfmicro_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: overrides I/O ports $0060-$0064 (emulation mode)
}

void zfmicro_usb_device::device_start()
{
	pci_device::device_start();

	add_map(4096, M_MEM, FUNC(zfmicro_usb_device::map));

	// INTA#
	intr_pin = 1;
}


void zfmicro_usb_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	// DEVSEL# medium timing
	// Fast Back-to-Back Capable
	status = 0x0280;
	// unverified default, assume same as OpenHCI specs
	m_HcFmInterval = 0x2edf;
}
