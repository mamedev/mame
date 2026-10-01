// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP95C061 SERIAL CHANNEL

    One of the two on-chip channels: SCxBUF, SCxCR, SCxMOD and BRxCR.

    In 8-bit UART mode the channel shifts bits at the rate BRxCR asks for and
    drives TXD, so a driver connects it to a serial device directly.  That path
    is only taken when the driver has bound txd(); with it unbound the channel
    keeps the byte-granularity behaviour the core had before, handing whole
    bytes to tx_byte() and completing a transmit immediately, so drivers that
    do not model the pins are unaffected.

    I/O interface (synchronous) mode is always byte-granularity here.

***************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP95C061_SERIAL_H
#define MAME_CPU_TLCS900_TMP95C061_SERIAL_H

#pragma once

#include "diserial.h"


class tmp95c061_serial_device : public device_t, public device_serial_interface
{
public:
	tmp95c061_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// INTTX is 0x80 and INTRX is 0x08, the bits the channel's INTES register uses
	auto setint() { return m_setint_cb.bind(); }

	// bit-granularity: binding txd() is what selects this path
	auto txd() { return m_txd_cb.bind(); }
	void rxd_w(int state) { rx_w(state); }

	// byte-granularity, for a link modelled above the pins
	auto tx_byte() { return m_tx_byte_cb.bind(); }
	void rx_byte(uint8_t data);

	// PORT 8's function bit for this channel's TXD pin; in I/O interface mode
	// the peer clocks the byte out, so the byte path only emits when the
	// channel transmits of its own accord or the pin is enabled.
	void set_pin_enabled(bool en) { m_pin_enabled = en; }

	uint8_t scbuf_r() { return m_rx_data; }
	void scbuf_w(uint8_t data);
	uint8_t sccr_r();
	void sccr_w(uint8_t data) { m_cr = data; }
	uint8_t scmod_r() { return m_mod; }
	void scmod_w(uint8_t data);
	uint8_t brcr_r() { return m_brcr; }
	void brcr_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_callback() override { m_txd_cb(transmit_register_get_data_bit()); }
	virtual void tra_complete() override;

private:
	// SCxMOD bits 3:2 -- 8-bit UART is the only asynchronous mode this models
	static constexpr uint8_t MOD_SM_MASK = 0x0c;
	static constexpr uint8_t MOD_SM_UART8 = 0x08;
	static constexpr uint8_t MOD_RXE = 0x20;

	bool bit_mode() const { return !m_txd_cb.isunset() && (m_mod & MOD_SM_MASK) == MOD_SM_UART8; }
	void update_rate();

	devcb_write8     m_setint_cb;
	devcb_write_line m_txd_cb;
	devcb_write8     m_tx_byte_cb;

	uint8_t m_cr = 0;
	uint8_t m_mod = 0;
	uint8_t m_brcr = 0;
	uint8_t m_rx_data = 0;
	uint8_t m_tx_hold = 0;
	bool    m_tx_hold_full = false;
	bool    m_tx_busy = false;
	bool    m_pin_enabled = false;
};

DECLARE_DEVICE_TYPE(TMP95C061_SERIAL, tmp95c061_serial_device)

#endif // MAME_CPU_TLCS900_TMP95C061_SERIAL_H
