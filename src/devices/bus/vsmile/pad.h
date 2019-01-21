// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_VSMILE_PAD_H
#define MAME_BUS_VSMILE_PAD_H

#pragma once

#include "vsmile_ctrl.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> vsmile_pad_device

class vsmile_pad_device : public vsmile_ctrl_device_base
{
public:
	// construction/destruction
	vsmile_pad_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~vsmile_pad_device();

	DECLARE_INPUT_CHANGED_MEMBER(pad_joy_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pad_color_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pad_button_changed);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	// vsmile_ctrl_device_base implementation
	virtual void tx_complete() override;
	virtual void rx_complete(uint8_t data, bool cts) override;

private:
	void uart_tx_fifo_push(uint8_t data);

	TIMER_CALLBACK_MEMBER(handle_idle);

	required_ioport m_io_joy;
	required_ioport m_io_colors;
	required_ioport m_io_buttons;

	emu_timer *m_idle_timer;

	uint8_t m_ctrl_probe_history[2];
	uint8_t m_ctrl_probe_count;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_PAD, vsmile_pad_device)

#endif // MAME_BUS_VSMILE_PAD_H
