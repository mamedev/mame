// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    machine/lpci.h

    Legacy PCI bus

***************************************************************************/

#ifndef LPCI_H
#define LPCI_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef uint32_t (*pci_read_func)(device_t *pcibus, device_t *device, int function, int reg, uint32_t mem_mask);
typedef void (*pci_write_func)(device_t *pcibus, device_t *device, int function, int reg, uint32_t data, uint32_t mem_mask);

// ======================> pci_bus_legacy_device

class pci_bus_legacy_device :  public device_t
{
public:
	// construction/destruction
	pci_bus_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint64_t read_64be(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void write_64be(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));

	void set_busnum(int busnum) { m_busnum = busnum; }
	void set_father(const char *father) { m_father = father; }
	void set_device(int num, const char *tag, pci_read_func read_func, pci_write_func write_func) {
		m_devtag[num] = tag; m_read_callback[num] = read_func; m_write_callback[num] = write_func; }

	pci_bus_legacy_device *pci_search_bustree(int busnum, int devicenum, pci_bus_legacy_device *pcibus);
	void add_sibling(pci_bus_legacy_device *sibling, int busnum);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t               m_busnum;
	const char *        m_devtag[32];
	pci_read_func       m_read_callback[32];
	pci_write_func      m_write_callback[32];
	const char *        m_father;
	device_t *          m_device[32];
	pci_bus_legacy_device * m_siblings[8];
	uint8_t               m_siblings_busnum[8];
	int                 m_siblings_count;

	offs_t              m_address;
	int8_t                m_devicenum; // device number we are addressing
	int8_t                m_busnumber; // pci bus number we are addressing
	pci_bus_legacy_device * m_busnumaddr; // pci bus we are addressing
};

// device type definition
extern const device_type PCI_BUS_LEGACY;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PCI_BUS_LEGACY_ADD(_tag, _busnum) \
	MCFG_DEVICE_ADD(_tag, PCI_BUS_LEGACY, 0) \
	downcast<pci_bus_legacy_device *>(device)->set_busnum(_busnum);
#define MCFG_PCI_BUS_LEGACY_DEVICE(_devnum, _devtag, _configread, _configwrite) \
	downcast<pci_bus_legacy_device *>(device)->set_device(_devnum, _devtag,_configread,_configwrite);
#define MCFG_PCI_BUS_LEGACY_SIBLING(_father_tag) \
	downcast<pci_bus_legacy_device *>(device)->set_father(_father_tag);


#endif /* PCI_H */
