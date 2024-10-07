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
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	// PCI9050 I/O register space handlers
	uint32_t lasrr_r(offs_t offset);
	void lasrr_w(offs_t offset, uint32_t data);
	uint32_t eromrr_r();
	void eromrr_w(uint32_t data);
	uint32_t lasba_r(offs_t offset);
	void lasba_w(offs_t offset, uint32_t data);
	uint32_t eromba_r();
	void eromba_w(uint32_t data);
	uint32_t lasbrd_r(offs_t offset);
	void lasbrd_w(offs_t offset, uint32_t data);
	uint32_t erombrd_r();
	void erombrd_w(uint32_t data);
	uint32_t csbase_r(offs_t offset);
	void csbase_w(offs_t offset, uint32_t data);
	uint32_t intcsr_r();
	void intcsr_w(uint32_t data);
	uint32_t cntrl_r();
	void cntrl_w(uint32_t data);

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
