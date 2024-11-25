// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Option Board

**********************************************************************/

#ifndef MAME_BUS_BBC_1MHZBUS_CFA3000OPT_H
#define MAME_BUS_BBC_1MHZBUS_CFA3000OPT_H

#pragma once

#include "1mhzbus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cfa3000_opt_device

class cfa3000_opt_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	cfa3000_opt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;

private:
	required_ioport m_opt;
};


// device type definition
DECLARE_DEVICE_TYPE(CFA3000_OPT, cfa3000_opt_device)


#endif // MAME_BUS_BBC_1MHZBUS_CFA3000OPT_H
