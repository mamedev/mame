// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_UART_H
#define MAME_SINCLAIR_SPECNEXT_UART_H

#pragma once

#include "diserial.h"

//static constexpr unsigned TX_FIFO_SIZE = 64; name too generic for global namespace

class specnext_uart_device : public device_t, public device_buffered_serial_interface<64>
{
public:
	specnext_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_txd_callback() { return m_out_txd_cb.bind(); }
	auto out_rx_full_near_callback() { return m_out_rx_full_near_cb.bind(); }
	auto out_tx_empty_callback() { return m_out_tx_empty_cb.bind(); }

	u8 reg_r(offs_t reg);
	void reg_w(offs_t reg, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void received_byte(u8 byte) override;

private:
	static constexpr unsigned RX_FIFO_SIZE = 512;

	devcb_write_line    m_out_txd_cb;
	devcb_write_line    m_out_rx_full_near_cb;
	devcb_write_line    m_out_tx_empty_cb;

	u8 m_framing;
	u8 m_prescalar_msb; // u3
	u16 m_prescalar_lsb; // u14
	u8 m_rx_fifo[RX_FIFO_SIZE];
	u16 m_rx_head, m_rx_tail;
	bool m_rx_empty;
	bool m_rx_full_near; // 3/4+

	u8 dat_r();
	u8 status_reg_r();
	void clear_rx_fifo();
	void update_serial();
};


DECLARE_DEVICE_TYPE(SPECNEXT_UART, specnext_uart_device)

#endif // MAME_SINCLAIR_SPECNEXT_UART_H
