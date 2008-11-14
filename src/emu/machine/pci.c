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

#include "driver.h"
#include "memconv.h"
#include "machine/pci.h"

#define LOG_PCI	0

struct pci_device_entry
{
	struct pci_device_entry *next;
	int bus, device, function;
	struct pci_device_info callbacks;
};

static struct pci_device_entry *pci_devices;
static struct pci_device_entry *pci_current_device;
static UINT32 pci_address;



void pci_init(void)
{
	pci_devices = NULL;
	pci_current_device = NULL;
	pci_address = 0;
}



void pci_add_device(int bus, int device, const struct pci_device_info *devinfo)
{
	struct pci_device_entry *pfi;

	pfi = (struct pci_device_entry *) auto_malloc(sizeof(*pfi));
	pfi->next = pci_devices;
	pfi->bus = bus;
	pfi->device = device;
	pfi->callbacks = *devinfo;

	pci_devices = pfi;
}



READ32_HANDLER(pci_32le_r)
{
	UINT32 result = 0xFFFFFFFF;
	int function, reg;

	offset %= 2;

	switch(offset)
	{
		case 0:
			result = pci_address;
			break;

		case 1:
			if (pci_current_device && pci_current_device->callbacks.read_callback)
			{
				function = (pci_address >> 8) & 0x07;
				reg = (pci_address >> 0) & 0xFC;
				result = pci_current_device->callbacks.read_callback(function, reg, mem_mask);
			}
			break;
	}

	if (LOG_PCI)
	{
		logerror("pci_32le_r(): CPU #%d pc=0x%08X offset=%d result=0x%08X\n",
			cpunum_get_active(), (unsigned) cpu_get_reg(space->cpu, REG_PC), offset, result);
	}
	return result;
}



WRITE32_HANDLER(pci_32le_w)
{
	struct pci_device_entry *pfi;
	int bus, device, function, reg;

	offset %= 2;

	if (LOG_PCI)
	{
		logerror("pci_32le_w(): CPU #%d pc=0x%08X offset=%d data=0x%08X\n",
			cpunum_get_active(), (unsigned) cpu_get_reg(space->cpu, REG_PC), offset, data);
	}

	switch(offset)
	{
		case 0:
			pci_address = data;
			pfi = NULL;

			/* lookup current device */
			if (pci_address & 0x80000000)
			{
				bus = (pci_address >> 16) & 0xFF;
				device = (pci_address >> 11) & 0x1F;

				for (pfi = pci_devices; pfi; pfi = pfi->next)
				{
					if ((pfi->bus == bus) && (pfi->device == device))
						break;
				}
			}
			pci_current_device = pfi;
			break;

		case 1:
			if (pci_current_device && pci_current_device->callbacks.write_callback)
			{
				function = (pci_address >> 8) & 0x07;
				reg = (pci_address >> 0) & 0xFC;
				pci_current_device->callbacks.write_callback(function, reg, data, mem_mask);
			}
			break;

	}
}



READ64_HANDLER(pci_64be_r) { return read64be_with_32le_handler(pci_32le_r, space, offset, mem_mask); }
WRITE64_HANDLER(pci_64be_w) { write64be_with_32le_handler(pci_32le_w, space, offset, data, mem_mask); }
