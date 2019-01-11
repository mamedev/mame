// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel I82541 ethernet controller

#ifndef MAME_MACHINE_I82541_H
#define MAME_MACHINE_I82541_H

#include "pci.h"

#define MCFG_I82541PI_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, I82541, 0x8086107c, 0x05, 0x020000, _subdevice_id)

class i82541_device : public pci_device {
public:
	i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void registers_map(address_map &map);
	void flash_map(address_map &map);
	void registers_io_map(address_map &map);
};

DECLARE_DEVICE_TYPE(I82541, i82541_device)

#endif // MAME_MACHINE_I82541_H
