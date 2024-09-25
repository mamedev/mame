// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG Programmable Graphic Module

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TUGPGM_H
#define MAME_BUS_TANBUS_TUGPGM_H

#pragma once

#include "bus/tanbus/tanbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tugpgm_device :
	public device_t,
	public device_tanbus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	tanbus_tugpgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_ioport m_links;

	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TUGPGM, tanbus_tugpgm_device)


#endif // MAME_BUS_TANBUS_TUGPGM_H
