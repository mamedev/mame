// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese
#include "emu.h"
#include "pci-usb.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"


DEFINE_DEVICE_TYPE(USB_OHCI, usb_ohci_device, "usb_ohci", "USB 1.1 OHCI interface")
DEFINE_DEVICE_TYPE(USB_UHCI, usb_uhci_device, "usb_uhci", "USB 1.1 UHCI interface")
DEFINE_DEVICE_TYPE(USB_EHCI, usb_ehci_device, "usb_ehci", "USB 2.0 EHCI interface")

/*
 *
 * National/Compaq/Microsoft OHCI
 *
 */

usb_ohci_device::usb_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, USB_OHCI, tag, owner, clock)
{
}

void usb_ohci_device::device_start()
{
	pci_device::device_start();
	add_map(4096, M_MEM, FUNC(usb_ohci_device::map));
}

void usb_ohci_device::device_reset()
{
	pci_device::device_reset();
}

void usb_ohci_device::map(address_map &map)
{
}

/*
 *
 * Intel/VIA UHCI
 *
 */

usb_uhci_device::usb_uhci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
{
}


usb_uhci_device::usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: usb_uhci_device(mconfig, USB_UHCI, tag, owner, clock)
{
}

void usb_uhci_device::device_start()
{
	pci_device::device_start();

	skip_map_regs(4);
	add_map(32, M_IO, FUNC(usb_uhci_device::map));

	save_item(NAME(m_usbcmd));
	save_item(NAME(m_usbsts));
	save_item(NAME(m_usbintr));
	save_item(NAME(m_frnum));
	save_item(NAME(m_flbaseadd));
	save_item(NAME(m_sofmod));
	save_item(NAME(m_portsc));
}

void usb_uhci_device::device_reset()
{
	pci_device::device_reset();

	m_usbcmd = 0;
	m_usbsts = 0;
	m_usbintr = 0;
	m_frnum = 0;
	// flbaseadd is "undefined"
	m_sofmod = 0x40;
	m_portsc[0] = m_portsc[1] = 0x80;
}

void usb_uhci_device::map(address_map &map)
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

/*
 *
 * EHCI
 *
 */

usb_ehci_device::usb_ehci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, USB_EHCI, tag, owner, clock)
{
}

void usb_ehci_device::device_start()
{
	pci_device::device_start();
	add_map(1024, M_MEM, FUNC(usb_ehci_device::map));
}

void usb_ehci_device::device_reset()
{
	pci_device::device_reset();
}

void usb_ehci_device::map(address_map &map)
{
}
