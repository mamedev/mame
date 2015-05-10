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
	cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask);
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask);

	DECLARE_WRITE8_MEMBER( cirrus_42E8_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
private:
};


// device type definition
extern const device_type CIRRUS;

#endif /* CIRRUS_H */
