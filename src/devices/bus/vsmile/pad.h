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

class vsmile_pad_device : public device_t, public device_vsmile_ctrl_interface
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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_vsmile_ctrl_interface implementation
	virtual void cts_w(int state) override;
	virtual void data_w(uint8_t data) override;

private:
	static const device_timer_id TIMER_UART_TX = 0;
	static const device_timer_id TIMER_PAD = 1;

	enum
	{
		XMIT_STATE_IDLE       = 0,
		XMIT_STATE_RTS        = 1,
		XMIT_STATE_CTS        = 2
	};

	void uart_tx_fifo_push(uint8_t data);
	void handle_uart_tx();

	required_ioport m_io_joy;
	required_ioport m_io_colors;
	required_ioport m_io_buttons;

	bool m_ctrl_cts;
	uint8_t m_ctrl_probe_history[2];
	uint8_t m_ctrl_probe_count;
	uint8_t m_uart_tx_fifo[32]; // arbitrary size
	uint8_t m_uart_tx_fifo_start;
	uint8_t m_uart_tx_fifo_end;
	uint8_t m_uart_tx_fifo_count;
	emu_timer *m_uart_tx_timer;
	int m_uart_tx_state;

	emu_timer *m_pad_timer;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_PAD, vsmile_pad_device)

#endif // MAME_BUS_VSMILE_PAD_H
