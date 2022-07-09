// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS 7001 USB Host controller

    TODO:
    - Stub interface, to be improved;
    - PCI values omitted from docs, assumes same as OpenHCI;

**************************************************************************************************/

#include "emu.h"
#include "sis7001_usb.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_MAP    (1U << 2) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS7001_USB, sis7001_usb_device, "sis7001_usb", "SiS 7001 USB Host Controller")

sis7001_usb_device::sis7001_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS7001_USB, tag, owner, clock)

{

}

void sis7001_usb_device::device_add_mconfig(machine_config &config)
{

}

void sis7001_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

void sis7001_usb_device::io_map(address_map &map)
{
	// operational mode
	map(0x000, 0x003).lr32(NAME([]() { return 0x00000110; }));
	// ...
	// HcFmInterval (Windows OSes fails if this isn't r/w)
	map(0x034, 0x037).lrw32(
		NAME([this]() { return m_HcFmInterval; } ),
		NAME([this](offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_HcFmInterval); LOG("Write HcFmInterval %08x & %08x\n", data, mem_mask); })
	);
	// ...
	 // HcRhDescriptorA, writeable except for 0x4ff
	map(0x048, 0x04b).lr32(NAME([this]() { return 0x01000000 | m_downstream_ports; }));
	// ...
//  map(0x05c, 0x05c) last item for function 2, missing on function 3

	// legacy support mode (8-bit each)
//  map(0x100, 0x100) control, bit 0 enables emulation mode
//  map(0x104, 0x104) input
//  map(0x108, 0x108) output
//  map(0x10c, 0x10f) status
}

void sis7001_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: overrides I/O ports $0060-$0064 (emulation mode)
}

void sis7001_usb_device::device_start()
{
	pci_device::device_start();

	add_map(512, M_MEM, FUNC(sis7001_usb_device::io_map));

	// INTD#
	intr_pin = 4;
}


void sis7001_usb_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0000;
	m_HcFmInterval = 0;
}
