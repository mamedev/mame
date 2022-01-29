// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Hybrid Music 4000 Keyboard

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-4000-Keyboard/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_Music4000.html

**********************************************************************/


#ifndef MAME_BUS_BBC_USERPORT_M4000_H
#define MAME_BUS_BBC_USERPORT_M4000_H

#pragma once

#include "userport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_m4000_device

class bbc_m4000_device :
	public device_t,
	public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_m4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t pb_r() override;
	virtual void write_cb1(int state) override;
	virtual void write_cb2(int state) override;

private:
	required_ioport_array<8> m_kbd;

	int m_clk;
	int m_dsb;
	uint8_t m_out;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_M4000, bbc_m4000_device)


#endif // MAME_BUS_BBC_USERPORT_M4000_H
