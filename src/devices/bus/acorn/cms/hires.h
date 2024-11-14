// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS High Resolution Colour Graphics Card

**********************************************************************/


#ifndef MAME_BUS_ACORN_CMS_HIRES_H
#define MAME_BUS_ACORN_CMS_HIRES_H

#pragma once

#include "bus/acorn/bus.h"
#include "machine/timer.h"
#include "video/ef9365.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cms_hires_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	cms_hires_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(flash_rate);
	void colour_reg_w(uint8_t data);

	required_device<screen_device> m_screen;
	required_device<ef9365_device> m_gdp;

	int m_flash_state;
};


// device type definition
DECLARE_DEVICE_TYPE(CMS_HIRES, cms_hires_device)


#endif // MAME_BUS_ACORN_CMS_HIRES_H
