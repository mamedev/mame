// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    video/cirrus.h

    Cirrus SVGA card emulation (preliminary)

***************************************************************************/

#ifndef CIRRUS_H
#define CIRRUS_H

#include "bus/lpci/pci.h"

// ======================> cirrus_device

class cirrus_device : public device_t,
						public pci_device_interface
{
public:
		// construction/destruction
	cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	void cirrus_42E8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
extern const device_type CIRRUS;

#endif /* CIRRUS_H */
