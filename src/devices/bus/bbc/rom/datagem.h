// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Gemini DataGem ROM Carrier

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_DATAGEM_H
#define MAME_BUS_BBC_ROM_DATAGEM_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_datagem_device

class bbc_datagem_device : public device_t,
	public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_datagem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint32_t get_rom_size() override { return 0x4000; }
	virtual uint8_t read(offs_t offset) override;
	virtual void decrypt_rom() override;

private:
	uint8_t m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_DATAGEM, bbc_datagem_device)


#endif // MAME_BUS_BBC_ROM_DATAGEM_H
