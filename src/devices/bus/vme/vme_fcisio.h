// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_FCISIO_H
#define MAME_BUS_VME_VME_FCISIO_H

#pragma once

#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_FCISIO1, vme_fcisio1_card_device)

class vme_fcisio1_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_fcisio1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fcisio1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_READ8_MEMBER (config_rd);

	DECLARE_READ16_MEMBER (bootvect_r);

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER (not_implemented_r);
	DECLARE_WRITE8_MEMBER (not_implemented_w);

	void fcisio1_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_duscc0;
	required_device<duscc68562_device> m_duscc1;
	required_device<duscc68562_device> m_duscc2;
	required_device<duscc68562_device> m_duscc3;

	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;

	// Pointer to System ROMs needed by bootvect_r
	uint16_t  *m_sysrom;
};

#endif // MAME_BUS_VME_VME_FCISIO_H
