// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1111 16K RAM Expansion Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1111_H
#define MAME_BUS_VIC20_VIC1111_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1111_device

class vic1111_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1111, vic1111_device)

#endif // MAME_BUS_VIC20_VIC1111_H
