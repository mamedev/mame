// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio ROM card emulation

**********************************************************************/

#ifndef MAME_BUS_POFO_ROM_H
#define MAME_BUS_POFO_ROM_H

#pragma once

#include "ccm.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> portfolio_rom_card_device

class portfolio_rom_card_device :  public device_t,
							  public device_portfolio_memory_card_slot_interface
{
public:
	// construction/destruction
	portfolio_rom_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_portfolio_memory_card_slot_interface overrides
	virtual bool cdet() override { return 0; }

	virtual uint8_t nrdi_r(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(PORTFOLIO_ROM_CARD, portfolio_rom_card_device)

#endif // MAME_BUS_POFO_ROM_H
