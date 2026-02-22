// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_VT82C586B_USB_H
#define MAME_MACHINE_VT82C586B_USB_H

#pragma once

#include "pci.h"
#include "pci-usb.h"

class vt82c586b_usb_device : public usb_uhci_device
{
public:
	vt82c586b_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	u8 m_misc_control[2];
};

DECLARE_DEVICE_TYPE(VT82C586B_USB, vt82c586b_usb_device)

#endif // MAME_MACHINE_VT82C586B_USB_H
