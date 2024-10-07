// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 RAM Card emulation

**********************************************************************/

#ifndef MAME_BUS_COMX35_RAM_H
#define MAME_BUS_COMX35_RAM_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_ram_device

class comx_ram_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_comx_expansion_card_interface overrides
	virtual uint8_t comx_mrd_r(offs_t offset, int *extrom) override;
	virtual void comx_mwr_w(offs_t offset, uint8_t data) override;
	virtual void comx_io_w(offs_t offset, uint8_t data) override;

private:
	memory_share_creator<uint8_t> m_ram;

	int m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(COMX_RAM, comx_ram_device)


#endif // MAME_BUS_COMX35_RAM_H
