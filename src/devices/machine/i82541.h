// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel I82541 ethernet controller

#ifndef I82541_H
#define I82541_H

#include "pci.h"

#define MCFG_I82541PI_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, I82541, 0x8086107c, 0x05, 0x020000, _subdevice_id)

class i82541_device : public pci_device {
public:
	i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(registers_map, 32);
	DECLARE_ADDRESS_MAP(flash_map, 32);
	DECLARE_ADDRESS_MAP(registers_io_map, 32);
};

extern const device_type I82541;

#endif
