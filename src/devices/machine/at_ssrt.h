// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_MACHINE_AT_SSRT_H
#define MAME_MACHINE_AT_SSRT_H

#pragma once

class at_ssrt_device
	: public device_t
{
public:
	at_ssrt_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	auto clk() { return m_clk_cb.bind(); } // clock output
	auto txd() { return m_txd_cb.bind(); } // data output

	auto pe() { return m_pe_cb.bind(); } // rx parity error
	auto rx() { return m_rx_cb.bind(); } // rx complete
	auto tx() { return m_tx_cb.bind(); } // tx complete

	void clk_w(int state); // clock input
	void rxd_w(int state); // data input

	// data register
	u8 data_r();
	void data_w(u8 data);

	// status helpers
	bool busy() const;
	bool rx_busy() const;
	bool tx_busy() const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void timer(s32 param);

private:
	devcb_write_line m_clk_cb;
	devcb_write_line m_txd_cb;

	devcb_write_line m_pe_cb;
	devcb_write_line m_rx_cb;
	devcb_write_line m_tx_cb;

	// state machine
	emu_timer *m_timer;
	u8 m_state;

	// rx/tx data
	u8 m_latch;
	u8 m_shift;
	bool m_parity;

	// input line state
	bool m_clk;
	bool m_rxd;
};

DECLARE_DEVICE_TYPE(AT_SSRT, at_ssrt_device)

#endif // MAME_MACHINE_AT_SSRT_H
