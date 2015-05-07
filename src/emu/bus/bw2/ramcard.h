// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 RAMCARD emulation

**********************************************************************/

#pragma once

#ifndef __BW2_RAMCARD__
#define __BW2_RAMCARD__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bw2_ramcard_device

class bw2_ramcard_device :  public device_t,
							public device_bw2_expansion_slot_interface
{
public:
	// construction/destruction
	bw2_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_bw2_expansion_slot_interface overrides
	virtual UINT8 bw2_cd_r(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6);
	virtual void bw2_cd_w(address_space &space, offs_t offset, UINT8 data, int ram2, int ram3, int ram4, int ram5, int ram6);
	virtual void bw2_slot_w(address_space &space, offs_t offset, UINT8 data);

private:
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;

	int m_en;
	UINT8 m_bank;
};


// device type definition
extern const device_type BW2_RAMCARD;



#endif
