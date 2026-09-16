// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANRAM (MT013 Iss2)

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANRAM_H
#define MAME_BUS_TANBUS_TANRAM_H

#pragma once

#include "bus/tanbus/tanbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tanram_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tanram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANRAM, tanbus_tanram_device)


#endif // MAME_BUS_TANBUS_TANRAM_H
