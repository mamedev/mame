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

class pci9050_device : public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *device, uint32_t clock);

	auto user_input_callback() { return m_user_input_handler.bind(); }
	auto user_output_callback() { return m_user_output_handler.bind(); }

	void set_map(int id, const address_map_constructor &map, device_t *device);

protected:
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_reset() override;

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
