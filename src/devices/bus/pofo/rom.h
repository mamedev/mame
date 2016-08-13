// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio ROM card emulation

**********************************************************************/

#pragma once

#ifndef __PORTFOLIO_ROM_CARD__
#define __PORTFOLIO_ROM_CARD__

#include "emu.h"
#include "ccm.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> portfolio_rom_card_t

class portfolio_rom_card_t :  public device_t,
				  			  public device_portfolio_memory_card_slot_interface
{
public:
	// construction/destruction
	portfolio_rom_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_portfolio_memory_card_slot_interface overrides
	virtual bool cdet() override { return 0; }

	virtual UINT8 nrdi_r(address_space &space, offs_t offset) override;
};


// device type definition
extern const device_type PORTFOLIO_ROM_CARD;



#endif
