// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP94C241 SERIAL

***************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP94C241_SERIAL_H
#define MAME_CPU_TLCS900_TMP94C241_SERIAL_H

#pragma once

class tmp94c241_device;

class tmp94c241_serial_device :
	public device_t
{
	static constexpr uint8_t PORT_F = 14; // 6 bit I/O. Shared with I/O functions of serial interface

public:
	tmp94c241_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t channel, uint32_t clock = 0);

	void TO2_trigger(int state);
	void serial_in(int state);
	auto serial_out() { return m_serial_out_cb.bind(); }
	void sck(int state);
	auto sck() { return m_sck_cb.bind(); }

	uint8_t brNcr_r();
	void brNcr_w(uint8_t data);
	void scNmod_w(uint8_t data);
	uint8_t scNmod_r();
	void scNcr_w(uint8_t data);
	uint8_t scNcr_r();
	void scNbuf_w(uint8_t data);
	uint8_t scNbuf_r();

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_serial();
	TIMER_CALLBACK_MEMBER(tx_timer_callback);

	emu_timer *m_tx_timer;

	uint8_t m_channel;
	uint8_t m_serial_control;
	uint8_t m_serial_mode;
	uint8_t m_baud_rate;
	uint32_t m_hz;

	uint8_t m_clock_count;
	uint8_t m_tx_shift_register;
	uint8_t m_serial_in;
	uint8_t m_serial_in_prev;
	uint8_t m_sck_in;

	uint8_t m_serial_out;
	uint8_t m_sck_out;

	devcb_write_line m_serial_out_cb;
	devcb_write_line m_sck_cb;

	tmp94c241_device *m_cpu;
};

DECLARE_DEVICE_TYPE(TMP94C241_SERIAL, tmp94c241_serial_device)

#endif // MAME_CPU_TLCS900_TMP94C241_SERIAL_H
