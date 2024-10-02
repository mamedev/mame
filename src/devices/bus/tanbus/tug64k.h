// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG 64K Dynamic RAM Board

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TUG64K_H
#define MAME_BUS_TANBUS_TUG64K_H

#pragma once

#include "bus/tanbus/tanbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tug64k_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tug64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_ioport_array<2> m_dsw;
	std::unique_ptr<uint8_t[]> m_ram;

	bool ram_enabled(offs_t offset, int inhrom, int inhram, int be);
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TUG64K, tanbus_tug64k_device)


#endif // MAME_BUS_TANBUS_TUG64K_H
