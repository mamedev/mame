// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pci9050.h - PLX PCI9050 PCI to 4x Local Bus Bridge

    by R. Belmont

*********************************************************************/

#ifndef MAME_MACHINE_PCI9050_H
#define MAME_MACHINE_PCI9050_H

#pragma once

#include "machine/pci.h"

#define MCFG_PCI9050_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI9050, 0x10b59050, 0x01, 0x06800000, 0x10b59050)

#define MCFG_PCI9050_SET_MAP(id, map) \
	downcast<pci9050_device *>(device)->set_map(id, address_map_constructor(&map, #map, this), this);

#define MCFG_PCI9050_USER_INPUT_CALLBACK(_write) \
	devcb = &downcast<pci9050_device &>(*device).set_user_input_callback(DEVCB_##_write);

#define MCFG_PCI9050_USER_OUTPUT_CALLBACK(_read) \
	devcb = &downcast<pci9050_device &>(*device).set_user_output_callback(DEVCB_##_read);

class pci9050_device : public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *device, uint32_t clock);

	template <class Object> devcb_base &set_user_input_callback(Object &&cb) { return m_user_input_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_user_output_callback(Object &&cb) { return m_user_output_handler.set_callback(std::forward<Object>(cb)); }

	void set_map(int id, const address_map_constructor &map, device_t *device);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	void postload(void);

private:
	void map(address_map &map);

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

	const char *m_names[4];
	device_t *m_devices[4];
	address_map_constructor m_maps[4];

	uint32_t m_lasrr[4], m_lasba[4], m_lasbrd[4], m_csbase[4];
	uint32_t m_eromrr, m_eromba, m_erombrd, m_intcsr, m_cntrl;

	void remap_local(int id);
	void remap_rom();

	devcb_read32 m_user_input_handler;
	devcb_write32 m_user_output_handler;
};

DECLARE_DEVICE_TYPE(PCI9050, pci9050_device)

#endif // MAME_MACHINE_PCI9050_H
