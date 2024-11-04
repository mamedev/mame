// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 80x25 VDU Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_80x25VDUIF.html

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_VDU80_H
#define MAME_BUS_ACORN_SYSTEM_VDU80_H

#pragma once

#include "bus/acorn/bus.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_vdu80_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_vdu80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	required_memory_region m_chargen;
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport m_links;

	std::unique_ptr<uint8_t[]> m_videoram;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_VDU80, acorn_vdu80_device)


#endif // MAME_BUS_ACORN_SYSTEM_VDU80_H
