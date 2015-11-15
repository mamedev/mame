// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pci9050.h - PLX PCI9050 PCI to 4x Local Bus Bridge

    by R. Belmont

*********************************************************************/

#ifndef _PCI9050_H
#define _PCI9050_H

#include "machine/pci.h"

#define MCFG_PCI9050_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI9050, 0x10b59050, 0x01, 0x06800000, 0x10b59050)

#define MCFG_PCI9050_SET_MAP(id, map) \
	downcast<pci9050_device *>(device)->set_map(id, ADDRESS_MAP_NAME(map), #map, owner);

class pci9050_device :
	public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// PCI9050 I/O register space handlers
	DECLARE_READ32_MEMBER( lasrr_r  );
	DECLARE_WRITE32_MEMBER(lasrr_w  );
	DECLARE_READ32_MEMBER( eromrr_r );
	DECLARE_WRITE32_MEMBER(eromrr_w );
	DECLARE_READ32_MEMBER( lasba_r  );
	DECLARE_WRITE32_MEMBER(lasba_w  );
	DECLARE_READ32_MEMBER( eromba_r );
	DECLARE_WRITE32_MEMBER(eromba_w );
	DECLARE_READ32_MEMBER( lasbrd_r );
	DECLARE_WRITE32_MEMBER(lasbrd_w );
	DECLARE_READ32_MEMBER( erombrd_r);
	DECLARE_WRITE32_MEMBER(erombrd_w);
	DECLARE_READ32_MEMBER( csbase_r );
	DECLARE_WRITE32_MEMBER(csbase_w );
	DECLARE_READ32_MEMBER( intcsr_r );
	DECLARE_WRITE32_MEMBER(intcsr_w );
	DECLARE_READ32_MEMBER( cntrl_r  );
	DECLARE_WRITE32_MEMBER(cntrl_w  );

	void set_map(int id, address_map_constructor map, const char *name, device_t *device);

protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map, 32);
	DECLARE_ADDRESS_MAP(empty, 32);

	const char *m_names[4];
	device_t *m_devices[4];
	address_map_constructor m_maps[4];

	UINT32 m_lasrr[4], m_lasba[4], m_lasbrd[4], m_csbase[4];
	UINT32 m_eromrr, m_eromba, m_erombrd, m_intcsr, m_cntrl;

	void remap_local(int id);
	void remap_rom();

	template<int id> void map_trampoline(address_map &map, device_t &device) {
		m_maps[id](map, *m_devices[id]);
	}
};

extern const device_type PCI9050;

#endif
