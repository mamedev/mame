// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Mouse

**********************************************************************/

#ifndef MAME_BUS_BBC_JOYPORT_MOUSE_H
#define MAME_BUS_BBC_JOYPORT_MOUSE_H

#pragma once

#include "joyport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbcmc_mouse_device

class bbcmc_mouse_device :
	public device_t,
	public device_bbc_joyport_interface
{
public:
	// construction/destruction
	bbcmc_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t pb_r() override;

	TIMER_CALLBACK_MEMBER(update);

private:
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_buttons;

	// quadrature output
	int m_xdir, m_ydir;

	// internal quadrature state
	int m_x, m_y;
	int m_phase_x, m_phase_y;

	emu_timer *m_mouse_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(BBCMC_MOUSE, bbcmc_mouse_device)


#endif // MAME_BUS_BBC_JOYPORT_MOUSE_H
