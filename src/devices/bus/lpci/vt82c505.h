// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C505 PCI bridge

*/

#ifndef __VT82C505_H__
#define __VT82C505_H__

#include "pci.h"

class vt82c505_device :  public device_t,
							public pci_device_interface
{
public:
	// construction/destruction
	vt82c505_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 m_window_addr[3];
	UINT8 m_window_attr[3];
};

// device type definition
extern const device_type VT82C505;


#endif /* __VT82C505_H__ */
