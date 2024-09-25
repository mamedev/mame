// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco AdamLink 300 Baud Modem emulation

**********************************************************************/

#ifndef MAME_BUS_ADAM_ADAMLINK_H
#define MAME_BUS_ADAM_ADAMLINK_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adamlink_device

class adamlink_device :  public device_t,
							public device_adam_expansion_slot_card_interface
{
public:
	// construction/destruction
	adamlink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_adam_expansion_slot_card_interface overrides
	virtual uint8_t adam_bd_r(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;
	virtual void adam_bd_w(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) override;
};


// device type definition
DECLARE_DEVICE_TYPE(ADAMLINK, adamlink_device)

#endif // MAME_BUS_ADAM_ADAMLINK_H
