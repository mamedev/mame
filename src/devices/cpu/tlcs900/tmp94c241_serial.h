// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP94C241 SERIAL

***************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP94C241_SERIAL_H
#define MAME_CPU_TLCS900_TMP94C241_SERIAL_H

#pragma once


class tmp94c241_serial_device :
	public device_t
{
	static constexpr uint8_t PORT_F = 14; // 6 bit I/O. Shared with I/O functions of serial interface

public:
	tmp94c241_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	tmp94c241_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint8_t channel) :
		tmp94c241_serial_device(mconfig, tag, owner, clock)
	{
		set_channel(channel);
	}

	void set_channel(uint8_t channel) { m_channel = channel; }

	auto setint_cb() { return m_setint_cb.bind(); }

	auto txd() { return m_txd_cb.bind(); }
	auto sclk_in() { return m_sclk_in_cb.bind(); }
	auto sclk_out() { return m_sclk_out_cb.bind(); }
	auto tx_start() { return m_tx_start_cb.bind(); }  // Signals start of new byte transmission

	void rxd(int state);
	void sioclk(int state);

	uint8_t brNcr_r();
	void brNcr_w(uint8_t data);
	void scNmod_w(uint8_t data);
	uint8_t scNmod_r();
	void scNcr_w(uint8_t data);
	uint8_t scNcr_r();
	void scNbuf_w(uint8_t data);
	uint8_t scNbuf_r();

	void pffc_sclk_w(int state) { m_pffc_sclk = state ? 1 : 0; }

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_callback);

	devcb_write8 m_setint_cb;

	devcb_write_line m_txd_cb;
	devcb_write_line m_sclk_in_cb;
	devcb_write_line m_sclk_out_cb;
	devcb_write_line m_tx_start_cb;  // Signals start of new byte transmission

	emu_timer *m_timer;

	uint8_t m_channel;

	uint8_t m_pffc_sclk;
	uint8_t m_serial_control;
	uint8_t m_serial_mode;
	uint8_t m_baud_rate;
	uint32_t m_hz;

	uint8_t m_rx_clock_count;
	uint8_t m_rx_shift_register;
	uint8_t m_rx_buffer;
	uint8_t m_rxd;
	uint8_t m_rxd_prev;
	uint8_t m_sioclk_state;

	uint8_t m_tx_clock_count;
	uint8_t m_tx_shift_register;
	uint8_t m_txd;
	uint8_t m_sclk_out;
	bool m_tx_skip_first_falling;  // Skip first falling edge after buf write to let receiver sample bit 0
	bool m_tx_needs_trailing_edge; // Defer INTTX to trailing rising edge so receiver samples bit 7 first

	uint8_t m_tx_buffer;           // TX double buffer (holds next byte while shift register is busy)
	bool m_tx_buffer_full;         // True when m_tx_buffer contains data waiting to be loaded
};

DECLARE_DEVICE_TYPE(TMP94C241_SERIAL, tmp94c241_serial_device)

#endif // MAME_CPU_TLCS900_TMP94C241_SERIAL_H
