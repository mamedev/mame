// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS7001_USB_H
#define MAME_MACHINE_SIS7001_USB_H

#pragma once

#include "pci.h"

class sis7001_usb_device : public pci_device
{
public:
	sis7001_usb_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		int num_ports
	) : sis7001_usb_device(mconfig, tag, owner, clock)
	{
		// 0x0c0310 - Serial Bus Controller, USB, OpenHCI Host
		// Assume no rev.
		set_ids(0x10397001, 0x00, 0x0c0310, 0x00);
		// TODO: should really read from a std::list interface
		m_downstream_ports = num_ports;
	}
	sis7001_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::MEDIA; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;

private:
	u8 m_downstream_ports;
	u32 m_HcFmInterval = 0;
};

DECLARE_DEVICE_TYPE(SIS7001_USB, sis7001_usb_device)

#endif
