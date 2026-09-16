// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    VTech Laser Lightpen Interface

***************************************************************************/

#ifndef MAME_BUS_VTECH_IOEXP_LPEN_H
#define MAME_BUS_VTECH_IOEXP_LPEN_H

#pragma once

#include "ioexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_lpen_interface_device

class vtech_lpen_interface_device : public vtech_ioexp_device
{
public:
	// construction/destruction
	vtech_lpen_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::CONTROLS; }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	uint8_t lpen_r(offs_t offset);
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_LPEN_INTERFACE, vtech_lpen_interface_device)

#endif // MAME_BUS_VTECH_IOEXP_LPEN_H
