#ifndef PCI_SMBUS_H
#define PCI_SMBUS_H

#include "pci.h"

#define MCFG_SMBUS_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, SMBUS, _main_id, _revision, 0x0c0500, _subdevice_id)

class smbus_device : public pci_device {
public:
	smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type SMBUS;

#endif
