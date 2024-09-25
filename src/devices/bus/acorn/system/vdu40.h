// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 40 Column VDU Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_VDU.html

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_VDU40_H
#define MAME_BUS_ACORN_SYSTEM_VDU40_H

#pragma once

#include "bus/acorn/bus.h"
#include "video/saa5050.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_vdu40_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_vdu40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	void vsync_changed(int state);

	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<saa5050_device> m_trom;

	std::unique_ptr<uint8_t[]> m_videoram;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_VDU40, acorn_vdu40_device)


#endif // MAME_BUS_ACORN_SYSTEM_VDU40_H
