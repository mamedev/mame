// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX Mouse emulation

**********************************************************************/

#ifndef MAME_BUS_MSX_CTRL_MOUSE_H
#define MAME_BUS_MSX_CTRL_MOUSE_H

#pragma once

#include "ctrl.h"


class msx_mouse_device : public device_t,
							public device_msx_general_purpose_port_interface
{
public:
	msx_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_buttons;
	required_ioport m_port_mouse_x;
	required_ioport m_port_mouse_y;
	u16 m_data;
	u8 m_stat;
	u8 m_old_pin8;
	s16 m_mouse_x;
	s16 m_mouse_y;
	attotime m_last_pin8_change;
	attotime m_timeout;
};


DECLARE_DEVICE_TYPE(MSX_MOUSE, msx_mouse_device)


#endif // MAME_BUS_MSX_CTRL_MOUSE_H
