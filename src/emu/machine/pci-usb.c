// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci-usb.h"

const device_type USB_UHCI = &device_creator<usb_uhci_device>;
const device_type USB_EHCI = &device_creator<usb_ehci_device>;

DEVICE_ADDRESS_MAP_START(map, 32, usb_uhci_device)
ADDRESS_MAP_END

usb_uhci_device::usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, USB_UHCI, "USB 1.1 UHCI interface", tag, owner, clock, "usb_uhci", __FILE__)
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

DEVICE_ADDRESS_MAP_START(map, 32, usb_ehci_device)
ADDRESS_MAP_END

usb_ehci_device::usb_ehci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, USB_EHCI, "USB 2.0 EHCI interface", tag, owner, clock, "usb_ehci", __FILE__)
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
