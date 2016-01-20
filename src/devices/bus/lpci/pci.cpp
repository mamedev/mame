// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pci.c

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
#include "pci.h"

#define LOG_PCI 0

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PCI_BUS = &device_creator<pci_bus_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pci_bus_device - constructor
//-------------------------------------------------
pci_bus_device::pci_bus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PCI_BUS, "PCI Bus", tag, owner, clock, "pci_bus", __FILE__), m_busnum(0),
		m_father(nullptr), m_address(0), m_devicenum(0), m_busnumber(0), m_busnumaddr(nullptr)
{
	m_siblings_count = 0;
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

READ32_MEMBER( pci_bus_device::read )
{
	UINT32 result = 0xffffffff;
	int function, reg;

	offset %= 2;

	switch (offset)
	{
		case 0:
			result = m_address;
			break;

		case 1:
			if (m_devicenum != -1)
			{
				if (m_busnumaddr->m_device[m_devicenum] != nullptr)
				{
					function = (m_address >> 8) & 0x07;
					reg = (m_address >> 0) & 0xfc;
					result = m_busnumaddr->m_device[m_devicenum]->pci_read(m_busnumaddr, function, reg, mem_mask);
				}
			}
			break;
	}

	if (LOG_PCI)
		logerror("read('%s'): offset=%d result=0x%08X\n", tag().c_str(), offset, result);

	return result;
}



pci_bus_device *pci_bus_device::pci_search_bustree(int busnum, int devicenum, pci_bus_device *pcibus)
{
	int a;
	pci_bus_device *ret;

	if (pcibus->m_busnum == busnum)
	{
		return pcibus;
	}
	for (a = 0; a < pcibus->m_siblings_count; a++)
	{
		ret = pci_search_bustree(busnum, devicenum, pcibus->m_siblings[a]);
		if (ret != nullptr)
			return ret;
	}
	return nullptr;
}



WRITE32_MEMBER( pci_bus_device::write )
{
	offset %= 2;

	if (LOG_PCI)
		logerror("write('%s'): offset=%d data=0x%08X\n", tag().c_str(), offset, data);

	switch (offset)
	{
		case 0:
			m_address = data;

			/* lookup current device */
			if (m_address & 0x80000000)
			{
				int busnum = (m_address >> 16) & 0xff;
				int devicenum = (m_address >> 11) & 0x1f;
				m_busnumaddr = pci_search_bustree(busnum, devicenum, this);
				if (m_busnumaddr != nullptr)
				{
					m_busnumber = busnum;
					m_devicenum = devicenum;
				}
				else
					m_devicenum = -1;
				if (LOG_PCI)
					logerror("  bus:%d device:%d\n", busnum, devicenum);
			}
			break;

		case 1:
			if (m_devicenum != -1)
			{
				if (m_busnumaddr->m_device[m_devicenum] != nullptr)
				{
					int function = (m_address >> 8) & 0x07;
					int reg = (m_address >> 0) & 0xfc;
					m_busnumaddr->m_device[m_devicenum]->pci_write(m_busnumaddr, function, reg, data, mem_mask);
				}
				if (LOG_PCI)
					logerror("  function:%d register:%d\n", (m_address >> 8) & 0x07, (m_address >> 0) & 0xfc);
			}
			break;
	}
}



READ64_MEMBER(pci_bus_device::read_64be)
{
	UINT64 result = 0;
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read(space, offset * 2 + 0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read(space, offset * 2 + 1, mem_mask >> 32) << 32;
	return FLIPENDIAN_INT64(result);
}

WRITE64_MEMBER(pci_bus_device::write_64be)
{
	data = FLIPENDIAN_INT64(data);
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	if (ACCESSING_BITS_0_31)
		write(space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_32_63)
		write(space, offset * 2 + 1, data >> 32, mem_mask >> 32);
}


void pci_bus_device::add_sibling(pci_bus_device *sibling, int busnum)
{
	m_siblings[m_siblings_count] = sibling;
	m_siblings_busnum[m_siblings_count] = busnum;
	m_siblings_count++;
}


//-------------------------------------------------
//  device_post_load - handle updating after a
//  restore
//-------------------------------------------------

void pci_bus_device::device_post_load()
{
	if (m_devicenum != -1)
	{
		m_busnumaddr = pci_search_bustree(m_busnumber, m_devicenum, this);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pci_bus_device::device_start()
{
	/* store a pointer back to the device */
	m_devicenum = -1;

	char id[3];
	/* find all our devices */
	for (int i = 0; i < ARRAY_LENGTH(m_devtag); i++)
	{
		sprintf(id, "%d", i);
		pci_connector *conn = downcast<pci_connector *>(subdevice(id));
		if (conn!=nullptr)
			m_device[i] = conn->get_device();
		else
			m_device[i] = nullptr;
	}

	if (m_father != nullptr) {
		pci_bus_device *father = machine().device<pci_bus_device>(m_father);
		if (father)
			father->add_sibling(this, m_busnum);
	}

	/* register pci states */
	save_item(NAME(m_address));
	save_item(NAME(m_devicenum));
	save_item(NAME(m_busnum));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pci_bus_device::device_reset()
{
	/* reset the drive state */
	m_devicenum = -1;
	m_address = 0;
}

//-------------------------------------------------
//  pci_device_interface - constructor
//-------------------------------------------------

pci_device_interface::pci_device_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}

//-------------------------------------------------
//  ~pci_device_interface - destructor
//-------------------------------------------------

pci_device_interface::~pci_device_interface()
{
}


const device_type PCI_CONNECTOR = &device_creator<pci_connector>;


pci_connector::pci_connector(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PCI_CONNECTOR, "PCI device connector abstraction", tag, owner, clock, "pci_connector", __FILE__),
	device_slot_interface(mconfig, *this)
{
}

pci_connector::~pci_connector()
{
}

void pci_connector::device_start()
{
}

pci_device_interface *pci_connector::get_device()
{
	return dynamic_cast<pci_device_interface *>(get_card_device());
}
