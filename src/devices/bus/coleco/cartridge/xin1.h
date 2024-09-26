// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    ColecoVision X-in-1 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_XIN1_H
#define MAME_BUS_COLECO_XIN1_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> colecovision_xin1_cartridge_device

class colecovision_xin1_cartridge_device : public device_t,
												public device_colecovision_cartridge_interface
{
public:
	// construction/destruction
	colecovision_xin1_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_colecovision_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) override;

private:
	uint32_t m_current_offset;
};


// device type definition
DECLARE_DEVICE_TYPE(COLECOVISION_XIN1, colecovision_xin1_cartridge_device)


#endif // MAME_BUS_COLECO_XIN1_H
