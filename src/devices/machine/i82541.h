// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel I82541 ethernet controller

#ifndef MAME_MACHINE_I82541_H
#define MAME_MACHINE_I82541_H

#include "pci.h"

class i82541_device : public pci_device {
public:
	i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id)
		: i82541_device(mconfig, tag, owner, clock)
	{
		set_ids(0x8086107c, 0x05, 0x020000, subdevice_id);
	}
	i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void registers_map(address_map &map) ATTR_COLD;
	void flash_map(address_map &map) ATTR_COLD;
	void registers_io_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(I82541, i82541_device)

#endif // MAME_MACHINE_I82541_H
