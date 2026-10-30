// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert
/***************************************************************************

    Namco Touchscreen device for Funcube series

***************************************************************************/

#ifndef MAME_NAMCO_FUNCUBE_TOUCHSCREEN_H
#define MAME_NAMCO_FUNCUBE_TOUCHSCREEN_H

#pragma once

#include "diserial.h"

class funcube_touchscreen_device : public device_t,
									public device_serial_interface
{
public:
	funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_cb() { return m_tx_cb.bind(); }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_complete() override;
	virtual void tra_callback() override;

	TIMER_CALLBACK_MEMBER(read_buttons);

private:
	devcb_write_line m_tx_cb;
	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_btn;

	uint8_t m_button_state;
	int32_t m_serial_pos;
	uint8_t m_serial[4];
};

DECLARE_DEVICE_TYPE(FUNCUBE_TOUCHSCREEN, funcube_touchscreen_device)

#endif // MAME_NAMCO_FUNCUBE_TOUCHSCREEN_H
