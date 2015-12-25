// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef PCI_USB_H
#define PCI_USB_H

#include "pci.h"

#define MCFG_USB_UHCI_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, USB_UHCI, _main_id, _revision, 0x0c0300, _subdevice_id)

#define MCFG_USB_EHCI_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, USB_EHCI, _main_id, _revision, 0x0c0320, _subdevice_id)

class usb_uhci_device : public pci_device {
public:
	usb_uhci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

class usb_ehci_device : public pci_device {
public:
	usb_ehci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type USB_UHCI;
extern const device_type USB_EHCI;

#endif
