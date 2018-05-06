// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pci.h

    PCI bus

***************************************************************************/

#ifndef MAME_BUS_LPCI_PCI_H
#define MAME_BUS_LPCI_PCI_H

#pragma once
#include <forward_list>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class pci_bus_device;

// ======================> pci_device_interface

class pci_device_interface :  public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~pci_device_interface();

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) = 0;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) = 0;

	virtual void remap(int space_id, offs_t start, offs_t end) {}

	void set_pci_bus(pci_bus_device *bus) { m_pci_bus = bus; }

protected:
	pci_device_interface(const machine_config &mconfig, device_t &device);

	pci_bus_device *m_pci_bus;
};

class pci_connector_device : public device_t,
						public device_slot_interface
{
public:
	pci_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pci_connector_device();

	pci_device_interface *get_device();

protected:
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(PCI_CONNECTOR, pci_connector_device)

// ======================> pci_bus_device

class pci_bus_device :  public device_t
{
public:
	// construction/destruction
	pci_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	DECLARE_READ64_MEMBER( read_64be );
	DECLARE_WRITE64_MEMBER( write_64be );

	void set_busnum(int busnum) { m_busnum = busnum; }
	void set_father(const char *father) { m_father = father; }
	void set_device(int num, const char *tag) {
		m_devtag[num] = tag; }

	pci_bus_device *pci_search_bustree(int busnum, int devicenum, pci_bus_device *pcibus);
	void add_sibling(pci_bus_device *sibling, int busnum);

	void remap(int space_id, offs_t start, offs_t end);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t               m_busnum;

	const char *        m_devtag[32];
	pci_device_interface *m_device[32];

	const char *        m_father;
	pci_bus_device *    m_siblings[8];
	uint8_t               m_siblings_busnum[8];
	int                 m_siblings_count;

	offs_t              m_address;
	int8_t                m_devicenum; // device number we are addressing
	int8_t                m_busnumber; // pci bus number we are addressing
	pci_bus_device *    m_busnumaddr; // pci bus we are addressing
};

// device type definition
DECLARE_DEVICE_TYPE(PCI_BUS, pci_bus_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PCI_BUS_ADD(_tag, _busnum) \
	MCFG_DEVICE_ADD(_tag, PCI_BUS, 0) \
	downcast<pci_bus_device *>(device)->set_busnum(_busnum);
#define MCFG_PCI_BUS_DEVICE(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, PCI_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)

#define MCFG_PCI_BUS_SIBLING(_father_tag) \
	downcast<pci_bus_device *>(device)->set_father(_father_tag);


#endif // MAME_BUS_LPCI_PCI_H
