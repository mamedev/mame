// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor R8 mouse emulation

*********************************************************************/

#ifndef MAME_BUS_ABCKB_R8_H
#define MAME_BUS_ABCKB_R8_H

#pragma once



#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R8_TAG "r8"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> r8_device

class r8_device :  public device_t
{
public:
	// construction/destruction
	r8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

private:
	TIMER_CALLBACK_MEMBER(scan_mouse);

	emu_timer *m_mouse_timer;
	required_ioport m_mouse_b;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	struct
	{
		int phase;
		int x;
		int y;
		int prev_x;
		int prev_y;
		int xa;
		int xb;
		int ya;
		int yb;
	} m_mouse;
};


// device type definition
DECLARE_DEVICE_TYPE(R8, r8_device)


#endif // MAME_BUS_ABCKB_R8_H
