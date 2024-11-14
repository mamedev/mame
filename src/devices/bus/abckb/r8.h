// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor R8 mouse emulation

*********************************************************************/

#ifndef MAME_BUS_ABCKB_R8_H
#define MAME_BUS_ABCKB_R8_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_r8_device

class luxor_r8_device :  public device_t
{
public:
	// construction/destruction
	luxor_r8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(scan_mouse);

	emu_timer *m_mouse_timer;
	required_ioport m_mouse_b;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	int m_phase;
	int m_x;
	int m_y;
	int m_prev_x;
	int m_prev_y;
	int m_xa;
	int m_xb;
	int m_ya;
	int m_yb;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_R8, luxor_r8_device)


#endif // MAME_BUS_ABCKB_R8_H
