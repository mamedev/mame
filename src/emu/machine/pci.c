/***************************************************************************

    machine/pci.c

    PCI bus

    The PCI bus is a 32-bit bus introduced by Intel, so it is little endian

    Control word:
        bit 31:         Enable bit
        bits 30-24:     Reserved
        bits 23-16:     PCI bus number
        bits 15-11:     PCI device number
        bits 10- 8:     PCI function number
        bits  7- 0:     Offset address

    Standard PCI registers:
        0x00    2   Vendor ID
        0x02    2   Device ID
        0x04    2   PCI Command
        0x06    2   PCI Status
        0x08    1   Revision ID
        0x09    1   Programming Interface
        0x0A    1   Subclass Code
        0x0B    1   Class Code

    Class Code/Subclass Code/Programming Interface
        0x00XXXX    Pre-PCI 2.0 devices
        0x000000        Non-VGA device
        0x000101        VGA device
        0x01XXXX    Storage Controller
        0x010000        SCSI
        0x0101XX        IDE
        0x0102XX        Floppy
        0x0103XX        IPI
        0x0104XX        RAID
        0x0180XX        Other
        0x02XXXX    Network Card
        0x020000        Ethernet
        0x020100        Tokenring
        0x020200        FDDI
        0x020300        ATM
        0x028000        Other
        0x03XXXX    Display Controller
        0x030000        VGA
        0x030001        8514 Compatible
        0x030100        XGA
        0x038000        Other
        0x04XXXX    Multimedia
        0x040000        Video
        0x040100        Audio
        0x048000        Other
        0x05XXXX    Memory Controller
        0x050000        RAM
        0x050100        Flash
        0x058000        Other
        0x06XXXX    Bridge
        0x060000        Host/PCI
        0x060100        PCI/ISA
        0x060200        PCI/EISA
        0x060300        PCI/Micro Channel
        0x060400        PCI/PCI
        0x060500        PCI/PCMCIA
        0x060600        PCI/NuBus
        0x060700        PCI/CardBus
        0x068000        Other

    Information on PCI vendors can be found at http://www.pcidatabase.com/

***************************************************************************/

#include "emu.h"
#include "devconv.h"
#include "machine/pci.h"

#define LOG_PCI	0

typedef struct _pci_bus_state pci_bus_state;
struct _pci_bus_state
{
	running_device *	busdevice;
	const pci_bus_config *	config;
	running_device *	device[32];
	offs_t					address;
	INT8					devicenum;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE pci_bus_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == PCI_BUS);

	return (pci_bus_state *)downcast<legacy_device_base *>(device)->token();
}



READ32_DEVICE_HANDLER( pci_32le_r )
{
	pci_bus_state *pcibus = get_safe_token(device);
	UINT32 result = 0xffffffff;
	int function, reg;

	offset %= 2;

	switch (offset)
	{
		case 0:
			result = pcibus->address;
			break;

		case 1:
			if (pcibus->devicenum != -1)
			{
				pci_read_func read = pcibus->config->device[pcibus->devicenum].read_callback;
				if (read != NULL)
				{
					function = (pcibus->address >> 8) & 0x07;
					reg = (pcibus->address >> 0) & 0xfc;
					result = (*read)(device, pcibus->device[pcibus->devicenum], function, reg, mem_mask);
				}
			}
			break;
	}

	if (LOG_PCI)
		logerror("pci_32le_r('%s'): offset=%d result=0x%08X\n", device->tag(), offset, result);

	return result;
}



WRITE32_DEVICE_HANDLER( pci_32le_w )
{
	pci_bus_state *pcibus = get_safe_token(device);

	offset %= 2;

	if (LOG_PCI)
		logerror("pci_32le_w('%s'): offset=%d data=0x%08X\n", device->tag(), offset, data);

	switch (offset)
	{
		case 0:
			pcibus->address = data;

			/* lookup current device */
			if (pcibus->address & 0x80000000)
			{
				int busnum = (pcibus->address >> 16) & 0xff;
				int devicenum = (pcibus->address >> 11) & 0x1f;
				pcibus->devicenum = (busnum == pcibus->config->busnum) ? devicenum : -1;
			}
			break;

		case 1:
			if (pcibus->devicenum != -1)
			{
				pci_write_func write = pcibus->config->device[pcibus->devicenum].write_callback;
				if (write != NULL)
				{
					int function = (pcibus->address >> 8) & 0x07;
					int reg = (pcibus->address >> 0) & 0xfc;
					(*write)(device, pcibus->device[pcibus->devicenum], function, reg, data, mem_mask);
				}
			}
			break;
	}
}



READ64_DEVICE_HANDLER(pci_64be_r) { return read64be_with_32le_device_handler(pci_32le_r, device, offset, mem_mask); }
WRITE64_DEVICE_HANDLER(pci_64be_w) { write64be_with_32le_device_handler(pci_32le_w, device, offset, data, mem_mask); }





/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( pci_bus )
{
	pci_bus_state *pcibus = get_safe_token(device);
	int devicenum;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->baseconfig().static_config() == NULL);
	assert(downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config() != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* store a pointer back to the device */
	pcibus->config = (const pci_bus_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
	pcibus->busdevice = device;
	pcibus->devicenum = -1;

	/* find all our devices */
	for (devicenum = 0; devicenum < ARRAY_LENGTH(pcibus->device); devicenum++)
		if (pcibus->config->device[devicenum].devtag != NULL)
			pcibus->device[devicenum] = device->machine->device(pcibus->config->device[devicenum].devtag);

	/* register pci states */
	state_save_register_device_item(device, 0, pcibus->address);
	state_save_register_device_item(device, 0, pcibus->devicenum);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( pci_bus )
{
	pci_bus_state *pcibus = get_safe_token(device);

	/* reset the drive state */
	pcibus->devicenum = -1;
	pcibus->address = 0;
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( pci_bus )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(pci_bus_state);		break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(pci_bus_config);		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(pci_bus); break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(pci_bus);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "PCI Bus");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Peripherial Bus");		break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_DEVICE(PCI_BUS, pci_bus);
