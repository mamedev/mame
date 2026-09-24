// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telercas TMCE-200/TMCE-225 CMOS RAM card emulation

**********************************************************************/

#ifndef MAME_BUS_TMC600_RAM_H
#define MAME_BUS_TMC600_RAM_H

#pragma once

#include "euro.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tmce200_device

class tmce200_device : public device_t, public device_tmc600_eurobus_card_interface
{
public:
	// construction/destruction
	tmce200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	tmce200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
};


// ======================> tmce225_device

class tmce225_device : public tmce200_device
{
public:
	// construction/destruction
	tmce225_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definitions
DECLARE_DEVICE_TYPE(TMCE200, tmce200_device)
DECLARE_DEVICE_TYPE(TMCE225, tmce225_device)

#endif // MAME_BUS_TMC600_RAM_H
