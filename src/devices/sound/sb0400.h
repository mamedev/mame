// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Creative labs SB0400 Audigy2 Value

#ifndef SB0400_H
#define SB0400_H

#include "machine/pci.h"

#define MCFG_SB0400_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, SB0400, 0x11020008, 0x00, 0x040100, _subdevice_id)

class sb0400_device : public pci_device {
public:
	sb0400_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type SB0400;

#endif
