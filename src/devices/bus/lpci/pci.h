// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pci.h

    PCI bus

***************************************************************************/

#ifndef PCI_H
#define PCI_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class pci_bus_device;

// ======================> pci_device_interface

class pci_device_interface :  public device_slot_card_interface
{
public:
	// construction/destruction
	pci_device_interface(const machine_config &mconfig, device_t &device);
	virtual ~pci_device_interface();

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) = 0;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) = 0;
};

class pci_connector: public device_t,
						public device_slot_interface
{
public:
	pci_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pci_connector();

	pci_device_interface *get_device();

protected:
	virtual void device_start() override;
};

extern const device_type PCI_CONNECTOR;

// ======================> pci_bus_device

class pci_bus_device :  public device_t
{
public:
	// construction/destruction
	pci_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint64_t read_64be(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void write_64be(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));

	void set_busnum(int busnum) { m_busnum = busnum; }
	void set_father(const char *father) { m_father = father; }
	void set_device(int num, const char *tag) {
		m_devtag[num] = tag; }

	pci_bus_device *pci_search_bustree(int busnum, int devicenum, pci_bus_device *pcibus);
	void add_sibling(pci_bus_device *sibling, int busnum);

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
extern const device_type PCI_BUS;


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


#endif /* PCI_H */
