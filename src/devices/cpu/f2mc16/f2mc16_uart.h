// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series UART

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_UART_H
#define MAME_CPU_F2MC16_F2MC16_UART_H

#include "f2mc16_intc.h"

#pragma once

class f2mc16_uart_device :
	public device_t
{
public:
	f2mc16_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	f2mc16_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t rx_vector, uint8_t tx_vector);

	auto sck() { return m_sck_cb.bind(); }
	auto sck_hz() { return m_sck_hz_cb.bind(); }
	auto sot() { return m_sot_cb.bind(); }

	void internal_timer_hz(uint32_t hz);
	void internal_timer(int state);
	void sck_hz(uint32_t hz);
	void sck(int state);
	void sin(int state);

	uint8_t smr_r();
	void smr_w(uint8_t data);
	uint8_t scr_r();
	void scr_w(uint8_t data);
	uint8_t sidr_r();
	void sodr_w(uint8_t data);
	uint8_t ssr_r();
	void ssr_w(uint8_t data);
	void cdcr_w(uint8_t data);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	void update_serial();
	TIMER_CALLBACK_MEMBER(update_internal_timer);
	TIMER_CALLBACK_MEMBER(rx_timer_callback);
	TIMER_CALLBACK_MEMBER(tx_timer_callback);

	required_device<f2mc16_intc_device> m_intc;
	uint8_t m_rx_vector;
	uint8_t m_tx_vector;
	devcb_write_line m_sck_cb;
	devcb_write32 m_sck_hz_cb;
	devcb_write_line m_sot_cb;

	emu_timer *m_tx_timer;
	emu_timer *m_rx_timer;

	attotime m_sck_in_changed;
	attotime m_peripheral_clock_changed;
	attotime m_internal_timer_changed;
	attotime m_rx_start_time;
	attotime m_tx_start_time;
	uint32_t m_internal_timer_hz;
	uint32_t m_peripheral_clock_hz;
	uint32_t m_sck_in_hz;

	uint8_t m_rx_ticks;
	uint8_t m_tx_ticks;
	int8_t m_sck_in;
	int8_t m_sck_out;
	uint32_t m_sck_out_hz;
	int8_t m_sin;
	int8_t m_sin_prev;
	int8_t m_sot;
	uint32_t m_hz;
	uint8_t m_clock_count;
	int8_t m_tx_bit;
	int8_t m_tx_bits;
	uint16_t m_tx_shift;
	int8_t m_rx_bit;
	uint16_t m_rx_shift;
	uint8_t m_smr;
	uint8_t m_scr;
	uint8_t m_sidr;
	uint8_t m_sodr;
	uint8_t m_ssr;
	uint8_t m_cdcr;
};

DECLARE_DEVICE_TYPE(F2MC16_UART, f2mc16_uart_device)

#endif
