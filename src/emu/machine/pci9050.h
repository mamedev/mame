/*********************************************************************

    pci9050.h - PLX PCI9050 PCI to 4x Local Bus Bridge
 
	by R. Belmont
 
*********************************************************************/

#ifndef _PCI9050_H
#define _PCI9050_H

#include "machine/pci.h"

#define MCFG_PCI9050_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI9050, 0x10b59050, 0x01, 0x06800000, 0x10b59050)

class pci9050_device : 
	public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// PCI9050 I/O register space handlers
	DECLARE_READ32_MEMBER(reg_r);
	DECLARE_WRITE32_MEMBER(reg_w);

protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map, 32);

//	address_space_config m_as0_config, m_as1_config, m_as2_config, m_as3_config;
//	address_space *m_as0, *m_as1, *m_as2, *m_as3;

	UINT32 m_regs[0x54/4];
};

extern const device_type PCI9050;

#endif
