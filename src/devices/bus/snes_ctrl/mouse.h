// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES Mouse

**********************************************************************/

#pragma once

#ifndef __SNES_MOUSE__
#define __SNES_MOUSE__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_mouse_device

class snes_mouse_device : public device_t,
							public device_snes_control_port_interface
{
public:
	// construction/destruction
	snes_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sms_control_port_interface overrides
	virtual UINT8 read_pin4() override;
	virtual void write_strobe(UINT8 data) override;
	virtual void port_poll() override;

private:
	required_ioport m_buttons;
	required_ioport m_xaxis;
	required_ioport m_yaxis;
	int m_strobe;
	int m_idx;
	UINT32 m_latch;

	INT16 m_x, m_y, m_oldx, m_oldy;
	UINT8 m_deltax, m_deltay;
	int m_speed;
	int m_dirx, m_diry;
};


// device type definition
extern const device_type SNES_MOUSE;


#endif
