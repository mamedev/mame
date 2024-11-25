// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS 40/80 Video Terminal Card

**********************************************************************/


#ifndef MAME_BUS_ACORN_CMS_4080TERM_H
#define MAME_BUS_ACORN_CMS_4080TERM_H

#pragma once

#include "bus/acorn/bus.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/timer.h"
#include "video/ef9345.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cms_4080term_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	cms_4080term_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	TIMER_DEVICE_CALLBACK_MEMBER(update_scanline);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void bus_irq_w(int state);

	required_device<via6522_device> m_via;
	required_device<mos6551_device> m_acia;
	required_device<screen_device> m_screen;
	required_device<ef9345_device> m_ef9345;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;
};


// device type definition
DECLARE_DEVICE_TYPE(CMS_4080TERM, cms_4080term_device)


#endif // MAME_BUS_ACORN_CMS_4080TERM_H
