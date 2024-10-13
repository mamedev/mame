// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_USB_H
#define MAME_MACHINE_PCI_USB_H

#pragma once

#include "pci.h"

class usb_ohci_device : public pci_device {
public:
	usb_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: usb_ohci_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x0c0310, subdevice_id);
	}
	usb_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

class usb_uhci_device : public pci_device {
public:
	usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: usb_uhci_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x0c0300, subdevice_id);
	}
	usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

class usb_ehci_device : public pci_device {
public:
	usb_ehci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: usb_ehci_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x0c0320, subdevice_id);
	}
	usb_ehci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(USB_OHCI, usb_ohci_device)
DECLARE_DEVICE_TYPE(USB_UHCI, usb_uhci_device)
DECLARE_DEVICE_TYPE(USB_EHCI, usb_ehci_device)

#endif // MAME_MACHINE_PCI_USB_H
