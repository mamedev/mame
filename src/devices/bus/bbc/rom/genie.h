// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro ROM carrier boards

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_GENIE_H
#define MAME_BUS_BBC_ROM_GENIE_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_genie_device

class bbc_pmsgenie_device : public device_t,
	public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_pmsgenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual uint32_t get_rom_size() override { return 0x4000; }

private:
	uint8_t m_write_latch;
	uint8_t m_bank_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_PMSGENIE, bbc_pmsgenie_device)


#endif // MAME_BUS_BBC_ROM_GENIE_H
