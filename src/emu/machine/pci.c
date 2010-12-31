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
	device_t *	busdevice;
	const pci_bus_config *	config;
	device_t *	device[32];
	pci_bus_state *		siblings[8];
	UINT8				siblings_busnum[8];
	int					siblings_count;
	offs_t					address;
	INT8					devicenum; // device number we are addressing
	INT8					busnum; // pci bus number we are addressing
	pci_bus_state *		busnumaddr; // pci bus we are addressing
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a PCI bus
-------------------------------------------------*/

INLINE pci_bus_state *get_safe_token(device_t *device)
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
				pci_read_func read = pcibus->busnumaddr->config->device[pcibus->devicenum].read_callback;
				if (read != NULL)
				{
					function = (pcibus->address >> 8) & 0x07;
					reg = (pcibus->address >> 0) & 0xfc;
					result = (*read)(pcibus->busnumaddr->busdevice, pcibus->busnumaddr->device[pcibus->devicenum], function, reg, mem_mask);
				}
			}
			break;
	}

	if (LOG_PCI)
		logerror("pci_32le_r('%s'): offset=%d result=0x%08X\n", device->tag(), offset, result);

	return result;
}



static pci_bus_state *pci_search_bustree(int busnum, int devicenum, pci_bus_state *pcibus)
{
int a;
pci_bus_state *ret;

	if (pcibus->config->busnum == busnum)
	{
		return pcibus;
	}
	for (a = 0; a < pcibus->siblings_count; a++)
	{
		ret = pci_search_bustree(busnum, devicenum, pcibus->siblings[a]);
		if (ret != NULL)
			return ret;
	}
	return NULL;
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
				pcibus->busnumaddr = pci_search_bustree(busnum, devicenum, pcibus);
				if (pcibus->busnumaddr != NULL)
				{
					pcibus->busnum = busnum;
					pcibus->devicenum = devicenum;
				}
				else
					pcibus->devicenum = -1;
				if (LOG_PCI)
					logerror("  bus:%d device:%d\n", busnum, devicenum);
			}
			break;

		case 1:
			if (pcibus->devicenum != -1)
			{
				pci_write_func write = pcibus->busnumaddr->config->device[pcibus->devicenum].write_callback;
				if (write != NULL)
				{
					int function = (pcibus->address >> 8) & 0x07;
					int reg = (pcibus->address >> 0) & 0xfc;
					(*write)(pcibus->busnumaddr->busdevice, pcibus->busnumaddr->device[pcibus->devicenum], function, reg, data, mem_mask);
				}
				if (LOG_PCI)
					logerror("  function:%d register:%d\n", (pcibus->address >> 8) & 0x07, (pcibus->address >> 0) & 0xfc);
			}
			break;
	}
}



READ64_DEVICE_HANDLER(pci_64be_r) { return read64be_with_32le_device_handler(pci_32le_r, device, offset, mem_mask); }
WRITE64_DEVICE_HANDLER(pci_64be_w) { write64be_with_32le_device_handler(pci_32le_w, device, offset, data, mem_mask); }


int pci_add_sibling( running_machine *machine, char *pcitag, char *sibling )
{
	device_t *device1 = machine->device(pcitag);
	device_t *device2 = machine->device(sibling);
	pci_bus_state *pcibus1 = get_safe_token(device1);
	pci_bus_state *pcibus2 = get_safe_token(device2);
	pci_bus_config *config2;

	if ((device1 == NULL) || (device2 == NULL) || (pcibus1 == NULL) || (pcibus2 == NULL))
		return 0;
	if (pcibus1->siblings_count == 8)
		return 0;
	config2 = (pci_bus_config *)downcast<const legacy_device_config_base &>(device2->baseconfig()).inline_config();
	pcibus1->siblings[pcibus1->siblings_count] = get_safe_token(device2);
	pcibus1->siblings_busnum[pcibus1->siblings_count] = config2->busnum;
	pcibus1->siblings_count++;
	return 1;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/


static STATE_POSTLOAD( pci_bus_postload )
{
	pci_bus_state *pcibus = (pci_bus_state *)param;

	if (pcibus->devicenum != -1)
	{
		pcibus->busnumaddr = pci_search_bustree(pcibus->busnum, pcibus->devicenum, pcibus);
	}
}


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

	if (pcibus->config->father != NULL)
		pci_add_sibling(device->machine, (char *)pcibus->config->father, (char *)device->tag());

	/* register pci states */
	state_save_register_device_item(device, 0, pcibus->address);
	state_save_register_device_item(device, 0, pcibus->devicenum);
	state_save_register_device_item(device, 0, pcibus->busnum);

	state_save_register_postload(device->machine, pci_bus_postload, pcibus);
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
