// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef GF7600GS_H
#define GF7600GS_H

#include "machine/pci.h"

#define MCFG_GEFORCE_7600GS_ADD(_tag, _subdevice_id) \
	MCFG_AGP_DEVICE_ADD(_tag, GEFORCE_7600GS, 0x10de02e1, 0xa1, _subdevice_id)

class geforce_7600gs_device : public pci_device {
public:
	geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map1, 32);
	DECLARE_ADDRESS_MAP(map2, 32);
	DECLARE_ADDRESS_MAP(map3, 32);
};

extern const device_type GEFORCE_7600GS;

#endif
