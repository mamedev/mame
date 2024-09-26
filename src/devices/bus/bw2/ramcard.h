// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 RAMCARD emulation

**********************************************************************/

#ifndef MAME_BUS_BW2_RAMCARD_H
#define MAME_BUS_BW2_RAMCARD_H

#pragma once

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
	bw2_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_bw2_expansion_slot_interface overrides
	virtual uint8_t bw2_cd_r(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6) override;
	virtual void bw2_cd_w(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6) override;
	virtual void bw2_slot_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_ram;

	int m_en;
	uint8_t m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(BW2_RAMCARD, bw2_ramcard_device)



#endif // MAME_BUS_BW2_RAMCARD_H
