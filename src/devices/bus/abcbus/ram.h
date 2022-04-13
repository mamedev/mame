// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ABC 80 16 KB RAM expansion card emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_RAM_H
#define MAME_BUS_ABCBUS_RAM_H

#pragma once

#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_16kb_ram_card_device

class abc80_16kb_ram_card_device : public device_t, public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc80_16kb_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override { }
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;
	virtual void abcbus_xmemw(offs_t offset, uint8_t data) override;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC80_16KB_RAM_CARD, abc80_16kb_ram_card_device)

#endif // MAME_BUS_ABCBUS_RAM_H
