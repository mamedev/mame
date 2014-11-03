#ifndef GF6800GT_H
#define GF6800GT_H

#include "machine/pci.h"

#define MCFG_GEFORCE_6800GT_ADD(_tag, _subdevice_id) \
	MCFG_AGP_DEVICE_ADD(_tag, GEFORCE_6800GT, 0x10de00f9, 0xa1, _subdevice_id)

class geforce_6800gt_device : public pci_device {
public:
	geforce_6800gt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map1, 32);
	DECLARE_ADDRESS_MAP(map2, 32);
	DECLARE_ADDRESS_MAP(map3, 32);
};

extern const device_type GEFORCE_6800GT;

#endif
