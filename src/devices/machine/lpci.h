// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    machine/lpci.h

    Legacy PCI bus

***************************************************************************/

#ifndef MAME_MACHINE_LPCI_H
#define MAME_MACHINE_LPCI_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pci_bus_legacy_device

class pci_bus_legacy_device :  public device_t
{
public:
	typedef device_delegate<uint32_t (int function, int reg, uint32_t mem_mask)> pci_bus_legacy_read_delegate;
	typedef device_delegate<void (int function, int reg, uint32_t data, uint32_t mem_mask)> pci_bus_legacy_write_delegate;

	// construction/destruction
	pci_bus_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint8_t busnum)
		: pci_bus_legacy_device(mconfig, tag, owner, clock)
	{
		set_busnum(busnum);
	}
	pci_bus_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	DECLARE_READ64_MEMBER( read_64be );
	DECLARE_WRITE64_MEMBER( write_64be );

	void set_busnum(int busnum) { m_busnum = busnum; }
	void set_father(const char *father) { m_father = father; }

	template <typename... T> void set_device_read(int num, T &&... args)  { m_read_callback[num] = pci_bus_legacy_read_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_device_write(int num, T &&... args) { m_write_callback[num] = pci_bus_legacy_write_delegate(std::forward<T>(args)...); }

	pci_bus_legacy_device *pci_search_bustree(int busnum, int devicenum, pci_bus_legacy_device *pcibus);
	void add_sibling(pci_bus_legacy_device *sibling, int busnum);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t           	m_busnum;
	pci_bus_legacy_read_delegate m_read_callback[32];
	pci_bus_legacy_write_delegate m_write_callback[32];
	const char *        m_father;
	pci_bus_legacy_device * m_siblings[8];
	uint8_t             m_siblings_busnum[8];
	int                 m_siblings_count;

	offs_t              m_address;
	int8_t              m_devicenum; // device number we are addressing
	int8_t              m_busnumber; // pci bus number we are addressing
	pci_bus_legacy_device * m_busnumaddr; // pci bus we are addressing
};

// device type definition
DECLARE_DEVICE_TYPE(PCI_BUS_LEGACY, pci_bus_legacy_device)


#endif // MAME_MACHINE_LPCI_H
