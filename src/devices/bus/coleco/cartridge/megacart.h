// license:BSD-3-Clause
// copyright-holders:Mark/Space Inc.
/**********************************************************************

    ColecoVision MegaCart emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_MEGACART_H
#define MAME_BUS_COLECO_MEGACART_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> colecovision_megacart_cartridge_device

class colecovision_megacart_cartridge_device : public device_t,
												public device_colecovision_cartridge_interface
{
public:
	// construction/destruction
	colecovision_megacart_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_colecovision_cartridge_interface overrides
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) override;

private:
	uint32_t m_bankcount;
	uint32_t m_activebank;
};


// device type definition
DECLARE_DEVICE_TYPE(COLECOVISION_MEGACART, colecovision_megacart_cartridge_device)


#endif // MAME_BUS_COLECO_MEGACART_H
