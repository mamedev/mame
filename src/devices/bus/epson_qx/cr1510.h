// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * ComFiler CR-1510 Card
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_CR1510_H
#define MAME_BUS_EPSON_QX_CR1510_H

#pragma once

#include "option.h"

#include "machine/wd1000.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson CR-1510 Device */

class cr1510_device : public device_t, public bus::epson_qx::device_option_expansion_interface
{
public:
	// construction/destruction
	cr1510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;
private:
	required_device<wd1000_device> m_hdd;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_CR1510, bus::epson_qx, cr1510_device)

#endif // MAME_BUS_EPSON_QX_CR1510_H
