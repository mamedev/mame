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
	// 2 USB ports
}

void i82371eb_usb_device::device_start()
{
	pci_device::device_start();

	skip_map_regs(4);
	add_map(32, M_IO, FUNC(i82371eb_usb_device::io_map));

	// INTD#
	intr_pin = 4;

	save_item(NAME(m_usbcmd));
	save_item(NAME(m_usbsts));
	save_item(NAME(m_usbintr));
	save_item(NAME(m_frnum));
	save_item(NAME(m_flbaseadd));
	save_item(NAME(m_sofmod));
	save_item(NAME(m_portsc));
}


void i82371eb_usb_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;

	m_usbcmd = 0;
	m_usbsts = 0;
	m_usbintr = 0;
	m_frnum = 0;
	// flbaseadd is "undefined"
	m_sofmod = 0x40;
	m_portsc[0] = m_portsc[1] = 0x80;
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
	map(0x00, 0x01).lrw16(
		NAME([this] () { return m_usbcmd; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_usbcmd);
			LOG("USBCMD %04x & %04x\n", data, mem_mask);
			LOG("MAXP %d CF %d SWDBG %d FGR %d EGSM %d GRESET %d HCRESET %d RS %d\n"
				, BIT(data, 7)
				, BIT(data, 6)
				, BIT(data, 5)
				, BIT(data, 4)
				, BIT(data, 3)
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
			if (mem_mask != 0xffff)
				LOG("\tnon-word access!\n");
		})
	);
	// --x- ---- HCHalted
	// ---x ---- Host Controller Process Error
	// ---- x--- Host System Error
	// ---- -x-- Resume Detect
	// ---- --x- USB Error Interrupt
	// ---- ---x USB Interrupt (USBINT)
	map(0x02, 0x03).lrw16(
		NAME([this] () { return m_usbsts; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (ACCESSING_BITS_0_7)
				m_usbsts &= ~(data & 0x3f);
		})
	);
	// USB Interrupt Enable
	// x--- Short Packet Interrupt Enable
	// -x-- Interrupt On Complete (IOC)
	// --x- Resume Interrupt Enable
	// ---x Time-out/CRC Interrupt Enable
	map(0x04, 0x05).lrw16(
		NAME([this] () { return m_usbintr; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_usbintr);
			LOG("USBINTR %04x & %04x\n", data, mem_mask);
		})
	);
	// Frame Number
	map(0x06, 0x07).lrw16(
		NAME([this] () { return m_frnum; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_frnum);
			LOG("FRNUM %04x & %04x\n", data, mem_mask);
			if (mem_mask != 0xffff)
				LOG("\tnon-word access!\n");
		})
	);
	// Frame List Base Address
	// bits 11-0 are <reserved>
	map(0x08, 0x0b).lrw32(
		NAME([this] () { return m_flbaseadd; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_flbaseadd);
			LOG("FLBASEADD %04x & %04x\n", data, mem_mask);
		})
	);
	// Start of Frame Modify
	map(0x0c, 0x0c).lrw8(
		NAME([this] () { return m_sofmod; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sofmod = data;
			LOG("SOFMOD %02x\n", data);
		})
	);
	// ---x ---- ---- ---- Suspend
	// ---- x--- ---- ---- Over-current Indicator Change
	// ---- -x-- ---- ---- Over-current Indicator (r/o)
	// ---- --x- ---- ---- Port Reset
	// ---- ---x ---- ---- Low Speed Device Attached (r/o)
	// ---- ---- 1--- ---- <reserved>, reads high
	// ---- ---- -x-- ---- Resume Detect
	// ---- ---- --xx ---- Line Status (r/o) D+/D-
	// ---- ---- ---- x--- Port Enable/Disable Change
	// ---- ---- ---- -x-- Port Enabled/Disabled
	// ---- ---- ---- --x- Connect Status Change
	// ---- ---- ---- ---x Current Connect Status (r/o)
	map(0x10, 0x13).lrw16(
		NAME([this] (offs_t offset) { return m_portsc[offset] | 0x80; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (ACCESSING_BITS_8_15)
			{
				m_portsc[offset] &= ~(0x1200);
				m_portsc[offset] |= (BIT(data, 12) << 12);
				m_portsc[offset] |= (BIT(data, 9) << 9);
				if (BIT(data, 11))
					m_portsc[offset] &= ~(1 << 11);
			}
			if (ACCESSING_BITS_0_7)
			{
				m_portsc[offset] &= ~(0x44);
				m_portsc[offset] |= (BIT(data, 6) << 6);
				m_portsc[offset] |= (BIT(data, 2) << 2);
				if (BIT(data, 3))
					m_portsc[offset] &= ~(1 << 3);
				if (BIT(data, 1))
					m_portsc[offset] &= ~(1 << 1);
			}
			LOG("PORTSC%d %04x & %04x -> %04x\n", offset, data, mem_mask, m_portsc[offset]);
			if (mem_mask != 0xffff)
				LOG("\tnon-word access!\n");
		})
	);
}

void i82371eb_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  io_space->install_device(0, 0x03ff, *this, &i82371eb_usb_device::io_map);
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
