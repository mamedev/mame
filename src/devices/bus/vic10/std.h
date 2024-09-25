// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC10_STD_H
#define MAME_BUS_VIC10_STD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_standard_cartridge_device

class vic10_standard_cartridge_device :  public device_t,
											public device_vic10_expansion_card_interface
{
public:
	// construction/destruction
	vic10_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_vic10_expansion_card_interface overrides
	virtual uint8_t vic10_cd_r(offs_t offset, uint8_t data, int lorom, int uprom, int exram) override;
	virtual void vic10_cd_w(offs_t offset, uint8_t data, int lorom, int uprom, int exram) override;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC10_STD, vic10_standard_cartridge_device)

#endif // MAME_BUS_VIC10_STD_H
