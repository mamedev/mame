/***************************************************************************

    machine/pci.h

    PCI bus

***************************************************************************/

#ifndef PCI_H
#define PCI_H

struct pci_device_info
{
	UINT32 (*read_callback)(int function, int reg, UINT32 mem_mask);
	void (*write_callback)(int function, int reg, UINT32 data, UINT32 mem_mask);
};


void pci_init(void);
void pci_add_device(int bus, int device, const struct pci_device_info *devinfo);

READ32_HANDLER(pci_32le_r);
WRITE32_HANDLER(pci_32le_w);

READ64_HANDLER(pci_64be_r);
WRITE64_HANDLER(pci_64be_w);

#endif /* PCI_H */


