// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

PIIX4E USB interface (UHCI)

TODO:
- Actual USB ports;

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_usb.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_USB, i82371eb_usb_device, "i82371eb_usb", "Intel 82371EB PIIX4E USB Host Controller")

i82371eb_usb_device::i82371eb_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82371EB_USB, tag, owner, clock)

{
	// 0x0c0300 - Serial Bus Controller, USB, UHCI Host
	// rev PIIX4E A-0 / PIIX4M A-0 = 0x01
	set_ids(0x80867112, 0x01, 0x0c0300, 0x00);
}

void i82371eb_usb_device::device_add_mconfig(machine_config &config)
{

}

void i82371eb_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// 0x60 sbrnum - serial bus release number
	// 0xc0-0xc1 legsup - Legacy Support Register
	map(0x60, 0xff).rw(FUNC(i82371eb_usb_device::unmap_log_r), FUNC(i82371eb_usb_device::unmap_log_w));
}

void i82371eb_usb_device::io_map(address_map &map)
{

}

void i82371eb_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  io_space->install_device(0, 0x03ff, *this, &i82371eb_usb_device::io_map);
}

void i82371eb_usb_device::device_start()
{
	pci_device::device_start();

	skip_map_regs(4);
	add_map(32, M_IO, FUNC(i82371eb_usb_device::io_map));

	// INTD#
	intr_pin = 4;
}


void i82371eb_usb_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;
}

/*
 * Debugging
 */

u8 i82371eb_usb_device::unmap_log_r(offs_t offset)
{
	LOGTODO("I82371EB_USB Unemulated [%02x] R\n", offset + 0x60);
	return 0;
}

void i82371eb_usb_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("I82371EB_USB Unemulated [%02x] %02x W\n", offset + 0x60, data);
}
