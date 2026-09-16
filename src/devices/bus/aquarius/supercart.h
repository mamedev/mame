// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Aquarius SuperCart Cartridge

***************************************************************************/

#ifndef MAME_BUS_AQUARIUS_SUPERCART_H
#define MAME_BUS_AQUARIUS_SUPERCART_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aquarius_sc1_device

class aquarius_sc1_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_ce_r(offs_t offset) override;
	virtual void mreq_ce_w(offs_t offset, uint8_t data) override;

private:
	uint8_t m_bank;
	int m_mode;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_SC1, aquarius_sc1_device)


#endif // MAME_BUS_AQUARIUS_SUPERCART_H
