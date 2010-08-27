/***************************************************************************

    machine/pci.h

    PCI bus

***************************************************************************/

#ifndef PCI_H
#define PCI_H

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT32 (*pci_read_func)(running_device *pcibus, running_device *device, int function, int reg, UINT32 mem_mask);
typedef void (*pci_write_func)(running_device *pcibus, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask);

typedef struct _pci_device_entry pci_device_entry;
struct _pci_device_entry
{
	const char *		devtag;
	pci_read_func		read_callback;
	pci_write_func		write_callback;
};

typedef struct _pci_bus_config pci_bus_config;
struct _pci_bus_config
{
	UINT8				busnum;
	pci_device_entry	device[32];
	const char *		father;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PCI_BUS_ADD(_tag, _busnum) \
	MDRV_DEVICE_ADD(_tag, PCI_BUS, 0) \
	MDRV_DEVICE_CONFIG_DATA32(pci_bus_config, busnum, _busnum)

#define MDRV_PCI_BUS_DEVICE(_devnum, _devtag, _configread, _configwrite) \
	MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(pci_bus_config, device, _devnum, pci_device_entry, devtag, _devtag) \
	MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(pci_bus_config, device, _devnum, pci_device_entry, read_callback, _configread) \
	MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(pci_bus_config, device, _devnum, pci_device_entry, write_callback, _configwrite)

#define MDRV_PCI_BUS_SIBLING(_father_tag) \
	MDRV_DEVICE_CONFIG_DATAPTR(pci_bus_config, father, _father_tag)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ32_DEVICE_HANDLER( pci_32le_r );
WRITE32_DEVICE_HANDLER( pci_32le_w );

READ64_DEVICE_HANDLER( pci_64be_r );
WRITE64_DEVICE_HANDLER( pci_64be_w );

int pci_add_sibling( running_machine *machine, char *pcitag, char *sibling );

/* ----- device interface ----- */

DECLARE_LEGACY_DEVICE(PCI_BUS, pci_bus);

#endif /* PCI_H */


