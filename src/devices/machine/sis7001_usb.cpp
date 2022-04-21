// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

	SiS 7001 USB Host controller

    TODO:
	- Stub interface, to be improved;
	- PCI values omitted from docs, assumes same as OpenHCI 

**************************************************************************************************/

#include "emu.h"
#include "sis7001_usb.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_MAP    (1U << 2) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_MAP)
#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(sis7001_usb, sis7001_usb_device, "sis7001_usb", "SiS 7001 USB Host Controller")

sis7001_usb_device::sis7001_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, sis7001_usb, tag, owner, clock)

{
	// 0x0c0310 - Serial Bus Controller, USB, OpenHCI Host
	// Assume no rev.
	set_ids(0x10397001, 0x00, 0x0c0310, 0x00);
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
	map(0x00, 0x00).lr8(NAME([this]() { return 0x00000110; }));
}

void sis7001_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//	io_space->install_device(0xd00, 0x0dff, *this, &sis7001_usb_device::io_map);
}

void sis7001_usb_device::device_start()
{
	pci_device::device_start();

#if 0
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
#endif
}


void sis7001_usb_device::device_reset()
{
	pci_device::device_reset();
	
	command = 0x0000;
	status = 0x0000;
}
