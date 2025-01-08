// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1010_H
#define MAME_BUS_VIC20_VIC1010_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1010_device

class vic1010_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	static constexpr unsigned MAX_SLOTS = 6;

	required_device_array<vic20_expansion_slot_device, MAX_SLOTS> m_expansion_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1010, vic1010_device)

#endif // MAME_BUS_VIC20_VIC1010_H
