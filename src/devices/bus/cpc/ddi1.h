// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * ddi1.h  --  Amstrad DDI-1 Floppy Disk Drive interface
 *
 * Provides uPD765A FDC, AMSDOS ROM, and 3" floppy disk drive.
 * CPC464 only, 664/6128/464+/6128+ already has this hardware built-in (AMSDOS is on the included Burnin' Rubber / BASIC cartridge for the 464+ and 6128+)
 *
 */

#ifndef MAME_BUS_CPC_DDI1_H
#define MAME_BUS_CPC_DDI1_H

#pragma once

#include "cpcexp.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"

class cpc_ddi1_device : public device_t, public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_ddi1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(motor_w);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(rombank_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void set_mapping(uint8_t type) override;
	virtual WRITE_LINE_MEMBER( romen_w ) override { m_romen = state; }

private:
	cpc_expansion_slot_device *m_slot;

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_connector;

	bool m_rom_active;
	bool m_romen;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_DDI1, cpc_ddi1_device)

#endif // MAME_BUS_CPC_DDI1_H
