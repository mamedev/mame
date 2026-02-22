// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

VT82C586B PCIC USB section

TODO:
- Actual USB ports;
- SMI traps for legacy support;
- devcb for RTCIREN in PIIX4 ISA, cfr. specification update;

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_usb.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C586B_USB, vt82c586b_usb_device, "vt82c586b_usb", "VT82C586B \"PIPC\" USB Host Controller")

vt82c586b_usb_device::vt82c586b_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: usb_uhci_device(mconfig, VT82C586B_USB, tag, owner, clock)
{
	// TODO: revision is a laconic "Silicon Revision Code", no value given
	set_ids(0x11063038, 0x41, 0x0c0300, 0x00000000);
	intr_pin = 0x04;
}

void vt82c586b_usb_device::device_add_mconfig(machine_config &config)
{
	// 2 USB ports
}

void vt82c586b_usb_device::device_start()
{
	usb_uhci_device::device_start();

	save_item(NAME(m_misc_control));
}


void vt82c586b_usb_device::device_reset()
{
	usb_uhci_device::device_reset();

	command = 0x0000;
	// Can be bus master, has address stepping and Memory Write and Invalidate
	// NOTE: also claims "Memory Space" capable (?)
	command_mask = 0x95;
	// medium DEVSEL#
	status = 0x0200;

	// claims default "16h" but then interrupt routing set to 0
	intr_line = 0;

	std::fill(std::begin(m_misc_control), std::end(m_misc_control), 0);
}

void vt82c586b_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// latency timer
	map(0x0d, 0x0d).lr8(NAME([] () { return 0x16; }));

	map(0x40, 0x40).lrw8(
		NAME([this] () { return m_misc_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_misc_control[0] = data & 0xef;

			// PCI Memory Command Option
			// if 1 disables Memory Read/Memory Read Multiple and Memory Write and Invalidate
			command_mask = BIT(data, 7) ? 0x85 : 0x95;
			command &= command_mask;
			LOG("40h: Misc Control 1 %02x\n", data);
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] () { return m_misc_control[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_misc_control[1] = data & 6;
			LOG("41h: Misc Control 2 %02x\n", data);
		})
	);
	// Serial Bus Release Number 1.0
	map(0x60, 0x60).lr8(NAME([] () { return 0x10; }));
	// Legacy Support
	// TODO: claims this register to be read only, where's the trap enable then? Same as i82371eb?
	// PCI.exe already contradicts with that statement, where it enables the equivalent of USBSMIEN
	map(0xc0, 0xc1).lrw16(
		NAME([] () { return 0x2000; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("C0h: writing Legacy Support (?) %04x & %04x\n", data, mem_mask);
		})
	);
}

void vt82c586b_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  io_space->install_device(0, 0x03ff, *this, &vt82c586b_usb_device::io_map);
}


