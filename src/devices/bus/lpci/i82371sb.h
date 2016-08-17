// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82371SB PCI IDE ISA Xcelerator (PIIX3)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef __I82371SB_H__
#define __I82371SB_H__

#include "pci.h"
#include "southbridge.h"

// ======================> i82371sb_device

class i82371sb_device :  public southbridge_device,
							public pci_device_interface
{
public:
	// construction/destruction
	i82371sb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT32 pci_isa_r(device_t *busdevice, int offset, UINT32 mem_mask);
	void pci_isa_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask);

	UINT32 pci_ide_r(device_t *busdevice, int offset, UINT32 mem_mask);
	void pci_ide_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask);

	UINT32 pci_usb_r(device_t *busdevice, int offset, UINT32 mem_mask);
	void pci_usb_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask);
private:
	UINT32 m_regs[3][0x400/4];
};

// device type definition
extern const device_type I82371SB;


#endif /* __I82371SB_H__ */
