// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "pci-usb.h"

DEFINE_DEVICE_TYPE(USB_OHCI, usb_ohci_device, "usb_ohci", "USB 1.1 OHCI interface")
DEFINE_DEVICE_TYPE(USB_UHCI, usb_uhci_device, "usb_uhci", "USB 1.1 UHCI interface")
DEFINE_DEVICE_TYPE(USB_EHCI, usb_ehci_device, "usb_ehci", "USB 2.0 EHCI interface")

void usb_ohci_device::map(address_map &map)
{
}

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

void usb_uhci_device::map(address_map &map)
{
}

usb_uhci_device::usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, USB_UHCI, tag, owner, clock)
{
}

void usb_uhci_device::device_start()
{
	pci_device::device_start();
	add_map(32, M_IO, FUNC(usb_uhci_device::map));
}

void usb_uhci_device::device_reset()
{
	pci_device::device_reset();
}

void usb_ehci_device::map(address_map &map)
{
}

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
