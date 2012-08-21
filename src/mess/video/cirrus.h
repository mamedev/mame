/***************************************************************************

    video/cirrus.h

    Cirrus SVGA card emulation (preliminary)

***************************************************************************/

#ifndef CIRRUS_H
#define CIRRUS_H

#include "machine/pci.h"

// ======================> cirrus_device

class cirrus_device : public device_t,
					  public pci_device_interface
{
public:
		// construction/destruction
    cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask);
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "cirrus"; }
private:
};


// device type definition
extern const device_type CIRRUS;

#endif /* CIRRUS_H */
