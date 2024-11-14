// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_SYS68K_ISIO_H
#define MAME_BUS_VME_SYS68K_ISIO_H

#pragma once

#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_SYS68K_ISIO1, vme_sys68k_isio1_card_device)

class vme_sys68k_isio1_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_sys68k_isio1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_isio1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	uint8_t config_rd();

	/* Dummy driver routines */
	uint8_t not_implemented_r();
	void not_implemented_w(uint8_t data);

	void fcisio1_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_duscc0;
	required_device<duscc68562_device> m_duscc1;
	required_device<duscc68562_device> m_duscc2;
	required_device<duscc68562_device> m_duscc3;

	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;

	required_memory_region m_eprom;
	required_shared_ptr<uint16_t> m_ram;
	memory_passthrough_handler m_boot_mph;
};

#endif // MAME_BUS_VME_SYS68K_ISIO_H
