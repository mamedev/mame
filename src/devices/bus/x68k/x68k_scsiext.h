// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_scsiext.h
 *
 *  Created on: 5/06/2012
 */

#ifndef MAME_BUS_X68K_X68K_SCSIEXT_H
#define MAME_BUS_X68K_X68K_SCSIEXT_H

#pragma once

#include "x68kexp.h"
#include "machine/mb89352.h"

class x68k_scsiext_device : public device_t,
							public device_x68k_expansion_card_interface
{
public:
	// construction/destruction
	x68k_scsiext_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(register_r);
	DECLARE_WRITE8_MEMBER(register_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_x68k_expansion_card_interface overrides
	virtual uint8_t iack2() override;

private:
	void irq_w(int state);
	void drq_w(int state);

	x68k_expansion_slot_device *m_slot;

	required_device<scsi_port_device> m_scsibus;
	required_device<mb89352_device> m_spc;
};

// device type definition
DECLARE_DEVICE_TYPE(X68K_SCSIEXT, x68k_scsiext_device)

#endif // MAME_BUS_X68K_X68K_SCSIEXT_H
