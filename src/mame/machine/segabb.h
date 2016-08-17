// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Lindbergh Sega baseboard

#ifndef SEGABB_H
#define SEGABB_H

#include "machine/pci.h"

#define MCFG_SEGA_LINDBERGH_BASEBOARD_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, SEGA_LINDBERGH_BASEBOARD, 0x105718c1, 0x10, 0x068000, 0x11db067b)

class sega_lindbergh_baseboard_device : public pci_device {
public:
	sega_lindbergh_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map1, 32);
	DECLARE_ADDRESS_MAP(map2, 32);
	DECLARE_ADDRESS_MAP(map3, 32);
};

extern const device_type SEGA_LINDBERGH_BASEBOARD;

#endif
