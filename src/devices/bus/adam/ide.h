// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Micro Innovations Powermate IDE Hard Disk emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_IDE__
#define __ADAM_IDE__

#include "emu.h"
#include "exp.h"
#include "machine/ataintf.h"
#include "machine/latch.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> powermate_ide_device

class powermate_ide_device :  public device_t,
	public device_adam_expansion_slot_card_interface
{
public:
	// construction/destruction
	powermate_ide_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_adam_expansion_slot_card_interface overrides
	virtual UINT8 adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;
	virtual void adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;

private:
	required_device<ata_interface_device> m_ata;
	required_device<output_latch_device> m_cent_data_out;

	UINT16 m_ata_data;
};


// device type definition
extern const device_type ADAM_IDE;



#endif
