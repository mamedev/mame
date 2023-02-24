// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************

	gew12_uart.h

	Yamaha GEW12 UART

*********************************************************************/

#ifndef MAME_MACHINE_GEW12_UART_H
#define MAME_MACHINE_GEW12_UART_H

#include "diserial.h"

class gew12_uart_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	gew12_uart_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	// configuration helpers
	auto tx_handler() { return m_tx_handler.bind(); }

	auto tx_irq_handler() { return m_tx_irq_handler.bind(); }
	auto rx_irq_handler() { return m_rx_irq_handler.bind(); }

	u8 status_r();
	u8 data_r();
	void data_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(rx_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	TIMER_CALLBACK_MEMBER(timer_tick);

private:
	devcb_write_line m_tx_handler;
	devcb_write_line m_tx_irq_handler;
	devcb_write_line m_rx_irq_handler;

	u8 m_data_out;
	u8 m_data_in;
	u8 m_status;
	u8 m_rx;

	emu_timer* m_timer;

	void irq_update();

	enum
	{
		// TODO: bits 0 and 1 are guessed - they're error bits but psr160/260 treats them identically
		STATUS_OVERRUN_ERROR = 0x1,
		STATUS_FRAMING_ERROR = 0x2,
		STATUS_ERROR_BITS    = (STATUS_OVERRUN_ERROR | STATUS_FRAMING_ERROR),
		STATUS_RX_READY      = 0x4,
		STATUS_RX_BITS       = (STATUS_ERROR_BITS | STATUS_RX_READY),
		STATUS_TX_READY      = 0x8
	};
};


DECLARE_DEVICE_TYPE(GEW12_UART, gew12_uart_device)

#endif // MAME_MACHINE_GEW12_UART_H
