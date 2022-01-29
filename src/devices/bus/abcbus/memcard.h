// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC Memory Card 55 10762-01 emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_MEMCARD_H
#define MAME_BUS_ABCBUS_MEMCARD_H

#pragma once

#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_memory_card_device

class abc_memory_card_device : public device_t, public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_memory_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override { }
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;

private:
	required_memory_region m_dos_rom;
	required_memory_region m_iec_rom;
	required_memory_region m_opt_rom;
	required_memory_region m_prn_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_MEMORY_CARD, abc_memory_card_device)



#endif // MAME_BUS_ABCBUS_MEMCARD_H
