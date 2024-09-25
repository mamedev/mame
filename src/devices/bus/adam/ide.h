// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Micro Innovations Powermate IDE Hard Disk emulation

**********************************************************************/

#ifndef MAME_BUS_ADAM_IDE_H
#define MAME_BUS_ADAM_IDE_H

#pragma once

#include "exp.h"
#include "bus/ata/ataintf.h"
#include "machine/output_latch.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> powermate_ide_device

class powermate_ide_device :  public device_t,
	public device_adam_expansion_slot_card_interface
{
public:
	// construction/destruction
	powermate_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_adam_expansion_slot_card_interface overrides
	virtual uint8_t adam_bd_r(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;
	virtual void adam_bd_w(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;

private:
	required_device<ata_interface_device> m_ata;
	required_device<output_latch_device> m_cent_data_out;

	uint16_t m_ata_data;
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_IDE, powermate_ide_device)

#endif // MAME_BUS_ADAM_IDE_H
