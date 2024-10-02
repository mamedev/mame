// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS IEEE Controller Board

**********************************************************************/


#ifndef MAME_BUS_ACORN_CMS_IEEE_H
#define MAME_BUS_ACORN_CMS_IEEE_H

#include "bus/acorn/bus.h"
#include "bus/ieee488/ieee488.h"
#include "machine/tms9914.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cms_ieee_device:
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	cms_ieee_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void bus_irq_w(int state);

	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
};


// device type definition
DECLARE_DEVICE_TYPE(CMS_IEEE, cms_ieee_device);


#endif /* MAME_BUS_ACORN_CMS_IEEE_H */
