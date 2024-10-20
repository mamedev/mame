// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtanic Real Time Clock

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANRTC_H
#define MAME_BUS_TANBUS_TANRTC_H

#pragma once

#include "tanbus.h"
#include "machine/mc146818.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tanrtc_device : public device_t, public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tanrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_device<mc146818_device> m_rtc;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANRTC, tanbus_tanrtc_device)


#endif // MAME_BUS_TANBUS_TANRTC_H
